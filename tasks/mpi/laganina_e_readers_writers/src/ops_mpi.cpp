
#include "mpi/laganina_e_readers_writers/include/ops_mpi.hpp"
#include<ctime>
#include <thread>
#include <vector>

bool laganina_e_readers_writers_mpi::TestMPITaskParallel::pre_processing() {
  internal_order_test();
  auto* ptr = reinterpret_cast<int*>(taskData->inputs[0]);
  iter = ptr[0];
  return true;
  
}

bool laganina_e_readers_writers_mpi::TestMPITaskParallel::validation() {
  internal_order_test();
  auto* ptr = reinterpret_cast<int*>(taskData->inputs[0]);
  int iter_ = ptr[0];
  return(iter_>=0);
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
        bool writerBusy = false;
		int numReaders =0;
     	// флаг работы писателя// флаг работы писателя
		
		//boost::mpi::timer timer;
		std::string source="";
		
		std::string requestType=""; 
	    bool terminate=false;
		int requestType_=0;
		int source_=0;
		int i=0;
        while ((i<iter)&&(!terminate)) {
		
			terminate=true;
			world.irecv(boost::mpi::any_source,2,source );
			if(source!=""){
			source_=std::stoi(source);}
         
            world.irecv(source_,1,requestType ); // получение сообщения от читателя или писателя
           
            if(requestType!=""){
            requestType_=std::stoi(requestType);}
			

            if (requestType_ == 1) { // Запрос на чтение
			    
                
                    // Отправка ответа  на чтение
					world.send(source_,3,!writerBusy);
                    world.send(source_, 4, sharedResource);
                    // Увеличиваем счетчик читателей
					if(!writerBusy){
                    numReaders++;
					}
                
            } 
			else if (requestType_ == 2) { // Запрос на запись
                if ((!writerBusy)&&(numReaders==0)) {
                    writerBusy = true;
				}
                    // Отправляем разрешение на запись
					world.send(source_, 5,(!writerBusy)&&(numReaders==0));
                    world.send(source_, 6,sharedResource);
                
            } 
			else if (requestType_ == 3) { // Завершение чтения
                numReaders--;
            } 
			else if (requestType_ == 4) {
				// Завершение записи
				std::string finishdata;
				world.recv(source_,7, finishdata);
				sharedResource=finishdata;
                writerBusy = false;
                
            }
			i++;
			terminate=false;
        }
		if(sharedResource.length()>=2){
		std::string rank_str=sharedResource.substr(sharedResource.length() - 3);
		res_=std::stoi(rank_str);
		}
		return true;
		
    } 
	else {
		int i=0;
        // Читатели и писатели (все процессы, кроме 0)
        int readerOrWriter = rank % 2; // 0 - читатель, 1 - писатель
        if (readerOrWriter == 0) { // Читатель
            while (i<iter) {
                // Запрос на чтение
                //int rank_=rank;
				std::string rank_= std::to_string(rank);
                world.send(0,2,rank_); 
				
                world.send(0, 1,std::to_string(1));
                          
                int allowed=1;
                world.recv(0,3,allowed);
                std::string requestMSg;
			    world.recv(0,4,requestMSg);
                
                if (allowed==1){
                    std::string data;
					data=requestMSg;
					
					world.send(0,2,rank_); 
				
					world.send(0, 1,std::to_string(3));
                }
                
                //Запрос завершения чтения
    
               i++; 
           }
        } 
		else { // Писатель
		
            while(i<iter){
                // Запрос на запись
                //int rank_=rank;
				std::string rank_= std::to_string(rank);
                world.send(0,2 ,rank_);
				
				world.send(0,1 ,std::to_string(2));
                
                int allowed=0;
                world.recv(0,5,allowed);
                //std::string requestMSg;
			    //world.recv(0,6,requestMSg);
                
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
				
                  
                    world.send(0, 2,rank_);

					world.send(0, 1,std::to_string(4));
					
					world.send(0,7,data);
					
                }
				i++;
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