#define _GNU_SOURCE

#include "parser.h"
#include "query.h"
#include "catalog.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <fstream>


// global objects
Catalog *cat;
Buffer_Manager *buf_mng;

time_t now();
//consider using the GNU Readline library instead of myreadline()
//it has a similar interface, and you'll get nice features like working
//cursor keys and (if initialized properly) history.
//run "man 3 readline" for more information
char* myreadline(char* prompt);

int main(int argc, char** argv){

   mkdir("data",0777);
   if(argc!=2){
	printf("Usage: %s <number of buffers>\n", *argv);
	printf("Buffers defaults to 3.\n");
	buf_mng=new Buffer_Manager();
   }else{
	int ret = strtol(*(argv+1), NULL,10);	
	if(ret==0 || ret < 0){
		printf("The number of buffers should be a positive integer!\n");
		return 0;
	}else{
		printf("Using %d Buffers\n", ret);
		buf_mng =new Buffer_Manager(ret);
	}
   }
   //load catalog from the "CATALOG" file
   cat=new Catalog();
   {
   	std::ifstream ifs("data/CATALOG");
	if(ifs.good()){
		boost::archive::text_iarchive ia(ifs);
		ia >> *cat;
	}else{
		fopen("data/CATALOG", "w");
	}
   }
   
   //Catalog catalog;
   char* input=NULL;
   statement_t* parsed=NULL;
   int stillrunning=1;
   int timing=0;
   while (stillrunning && (input=myreadline(">> ")) != 0){
      parsed=parse_statement(input);
      if (parsed){
	 time_t begintime=now();
	 /*
	  * dispatch_print is an example of how to work with the AST to detect
	  * and run the various commands, as well as how to get at all of the
	  * information in the AST. Remove this call when you implement the
	  * logic to actually run the commands in the DBMS.
	  * (You can use the functions in print.c for debugging purposes)
	  */
	 dispatch_query(parsed);	

	 if(parsed->set && parsed->set->variable == CONFIG_TIMER)
	    timing = parsed->set->value;
	 if(parsed->parameterless == CMD_EXIT){
	    stillrunning=0;
	 }
	 time_t endtime=now();
	 if (timing)
	    printf("Elapsed time: %dus\n", endtime-begintime);
      }else{
	 /* There was a syntax error, and the parser has already 
	  * printed an error message, so nothing to do here.*/
      }

      free(input);    
      input=NULL;
      free_statement(parsed);
      parsed=NULL;
   }

   {	
	//persisting catalog data
	std::ofstream ofs("data/CATALOG");
	boost::archive::text_oarchive oa(ofs);	
	oa << *cat;
   }
   delete buf_mng;
   delete cat;
   return 0;
}

char* myreadline(char* prompt){
   char* input=NULL;
   size_t inputlength=0;
   printf("%s",prompt);
   if (getline(&input, &inputlength, stdin) != -1){
      return input;
   }
   else{
      return input;
   }
}

time_t now(){
   struct timeval t;
   gettimeofday(&t,0);
   return t.tv_sec*1000000+t.tv_usec;
}
