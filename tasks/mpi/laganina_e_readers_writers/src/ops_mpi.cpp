
#include "mpi/laganina_e_readers_writers/include/ops_mpi.hpp"
#include<ctime>
#include <thread>
#include <vector>

bool laganina_e_readers_writers_mpi::TestMPITaskParallel::pre_processing() {
  internal_order_test();
  auto* ptr = reinterpret_cast<int*>(taskData->inputs[0]);
  time = ptr[0];
  return true;
  
}

bool laganina_e_readers_writers_mpi::TestMPITaskParallel::validation() {
  internal_order_test();
  auto* ptr = reinterpret_cast<int*>(taskData->inputs[0]);
  int time_ = ptr[0];
  return(time_>=0);
}

bool laganina_e_readers_writers_mpi::TestMPITaskParallel::run() {
  internal_order_test();
  
  int rank = world.rank();
  int size = world.size();
  if(size==1){
	  return true;
  }
  if (rank == 0) {
        std::string sharedResource = "";	
        bool writerBusy = false;        // флаг работы писателя
		int numReaders = 0;
		
		//boost::mpi::timer timer;
		
        while ((((float)clock()) / CLOCKS_PER_SEC )<=time) {
			int source;
			world.recv(boost::mpi::any_source,2,source );
			int requestType;
          
            world.recv(boost::mpi::any_source,1,requestType ); // получение сообщения от читателя или писателя
           
            
            
			

            if (requestType == 1) { // Запрос на чтение
			    
                
                    // Отправка ответа  на чтение
					world.send(source,3,!writerBusy);
                    world.send(source, 4, sharedResource);
                    // Увеличиваем счетчик читателей
					if(!writerBusy){
                    numReaders++;
					}
                
            } 
			else if (requestType == 2) { // Запрос на запись
                if ((!writerBusy)&&(numReaders==0)) {
                    writerBusy = true;
				}
                    // Отправляем разрешение на запись
					world.send(source, 5,(!writerBusy)&&(numReaders==0));
                    world.send(source, 6,sharedResource);
                
            } 
			else if (requestType == 3) { // Завершение чтения
                numReaders--;
            } 
			else if (requestType == 4) {
				// Завершение записи
				std::string finishdata;
				world.recv(source,7, finishdata);
				sharedResource=finishdata;
                writerBusy = false;
                
            }
        }
		
		std::string rank_str=sharedResource.substr(sharedResource.length() - 3);
		res_=std::stoi(rank_str);
		
		
    } 
	else {
        // Читатели и писатели (все процессы, кроме 0)
        int readerOrWriter = rank % 2; // 0 - читатель, 1 - писатель
        if (readerOrWriter == 0) { // Читатель
            while((((float)clock()) / CLOCKS_PER_SEC )<=time){
                // Запрос на чтение
                
                world.send(0,2,rank); 
                world.send(0, 1,1);
                          
                int allowed;
                world.recv(0,3,allowed);
                std::string requestMSg;
			    world.recv(0,4,requestMSg);
                
                if (allowed==1){
                    std::string data;
					data=requestMSg;
                }
                
                //Запрос завершения чтения
    
                world.send(0,2,rank); 
                world.send(0, 1,3);
            }
        } 
		else { // Писатель
            while((((float)clock()) / CLOCKS_PER_SEC )<=time){
                // Запрос на запись
     
                world.send(0,2 ,rank);
				world.send(0,1 ,2);
                
                int allowed;
                world.recv(0,5,allowed);
                std::string requestMSg;
			    world.recv(0,6,requestMSg);
                
                if(allowed==1){
					std::string data;
					if(rank<=9){
                     data = std::string("Data from writer ",17) + std::string("00",2) + std::to_string(rank);
					}
					if((rank>=10)&&(rank<=99)){
				     data = std::string("Data from writer ",17) + std::string("0",1) + std::to_string(rank);
					}
					if(rank>=100){
				     data = std::string("Data from writer ",17)  + std::to_string(rank);
					}
				
                    // Запись данных (в данном случае, просто обновление ресурса)
                 
                    // Запрос завершения записи
				
                  
                    world.send(0, 2,rank);

					world.send(0, 1,4);
					world.send(0,7,data);
					
                }
            }
        }
		
    }
	return true;
}
  


bool laganina_e_readers_writers_mpi::TestMPITaskParallel::post_processing() {
  internal_order_test();
  if(world.size()!=1){
	if(world.rank()==0){
		
		reinterpret_cast<int*>(taskData->outputs[0])[0]= res_;
		
	}
  }
  return true;
}