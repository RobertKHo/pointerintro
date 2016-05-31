// $Id: commands.cpp,v 1.11 2014-06-11 13:49:31-07 - - $
// Robert Ho rokho@ucsc.edu
// Daniel Urrutia deurruti@ucsc.edu

#include "commands.h"
#include "debug.h"
//create a map of commands
commands::commands(): map ({
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"#"     , fn_cmt   },
}){}

command_fn commands::at (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   command_map::const_iterator result = map.find (cmd);
   if (result == map.end()) {
      throw yshell_exn (cmd + ": no such function");
   }
   return result->second;
}

//prints contents of a file
//error if file is a directory
//or doesn't exist
void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   if(words.size() == 1){ 
      cerr << "cat:No such file specified" << endl;  
   }else{
      inode_ptr cwd = state.get_cwd(); 
      string filename = words.at(1); 
      cwd->find_file(cwd,filename);
   }

}
//changes current
//working directory
void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   inode_ptr cwd = state.get_cwd();
   
   string dirname = words.at(1); 
   cwd->find_dir(state,dirname);
   
}
//print user input
void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
  
    
   for(auto i = begin(words) + 1; i != end(words) ; ++i){   
      cout << *i << ' ';
     
   } 
  cout << endl;
   DEBUGF ('c', words);
}

//exits the shell
void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if(words.size() > 2){ 
       exit_status::set(127);
       throw ysh_exit_exn();
   } 

   for(size_t i = 1; i< words.size(); ++i )  { 
    
     try{
     size_t endpos; 
     int num = stoi(words[i],&endpos);
     

     if(endpos == words[i].size()){ 
       exit_status::set(num);
         }else{
        exit_status::set(127);
       }
     }catch (invalid_argument &error){ 
         exit_status::set(127); 
     }catch (out_of_range &error) {
         exit_status::set(127);
      }  
   } 
   state.reset_ptr();
   throw ysh_exit_exn();
}
//prints the contents of
//the specified directory
void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
    inode_ptr tmp;
    string filename = "";
    size_t counter = 0;
    wordvec input;

    if(words.size() >1){ 
    string slash = words.at(1);
    counter = count(slash.begin(),slash.end(),'/');
    input = split(slash,"/");
    //cout << input.size() << "  "<<counter <<endl;
    }  

    //cout << input.size() << "  "<<counter <<endl;
     
    if(words.size() == 1){
       //cout << "here" << endl;
       tmp = state.get_cwd();
    }else if(input.size() == counter){
       //cout << "hello?" << endl;
       filename = words.at(1);
       tmp = state.get_base();
    } else if(input.size() != counter){
       //cout << "hey" << endl;
       filename = words.at(1);
       if(filename == "/"){ 
          tmp = state.get_base();
          }else{
          tmp = state.get_cwd();
          }
      }
     tmp->ls(tmp, filename,input);
}
//recursively call ls
void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
    inode_ptr tmp;
    string filename = "";
    size_t counter = 0;
    wordvec input;

    if(words.size() >1){ 
    string slash = words.at(1);
    counter = count(slash.begin(),slash.end(),'/');
    input = split(slash,"/");
    //cout << input.size() << "  "<<counter <<endl;
    }  

    //cout << input.size() << "  "<<counter <<endl;
     
    if(words.size() == 1){
       tmp = state.get_cwd();
       cout<< "/:" << endl;
    }else if(input.size() == counter){
       filename = words.at(1);
       tmp = state.get_base();
    } else if(input.size() != counter){
       filename = words.at(1);
       if(filename == "/"){ 
          tmp = state.get_base();
          }else{
          tmp = state.get_cwd();
          }
      }
     tmp->lsr(tmp, filename,input);
}

//make a file with a string
void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   //pre: check if words contains a specified directory
   //inode_ptr tmp; 
   //wordvec s;
   //string filename;
   //string sub;
   //size_t n = 0; 
   //size_t spos = 0;
   //for(auto i = begin(words) +1; i != end(words); ++i){
   //    filename.append(*i);
   //    filename.append(" ");   
   //   }
   //   cout << filename << endl; 
   //if(words.size() >= 1){ 
   //   string slash = words.at(1);
   //   spos = filename.find_last_of("/");
   //   sub = filename.substr(spos+1,string::npos); 
   //   n = count(slash.begin(),slash.end(),'/');
   //   s = split(slash,"/");
   //   s.pop_back();
   //   cout << s.size() << " " << n << endl;
   //   }
        //cout << s.at(s.size() -1) << endl;     
   //     cout << sub << endl;  
   // if(n >0){ 
   //    tmp = state.get_base(); 
   //    }else if(n == 0){
   //    cout << "here" << endl;
   //    tmp = state.get_cwd();
   //    }
   //    tmp->make(tmp,sub,s);
   inode_ptr cwd = state.get_cwd();
   wordvec input(words.begin()+1,words.end());
   cwd->make(cwd,input); 

}    
//make a directory
void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() == 1) cerr << "mkdir: No such file or directory"
                              << " specified" << endl;
   string filename = words.at(1);
   inode_ptr ptr = state.get_cwd();
   ptr->mkdir(ptr,filename);
}

//change the shell prompt
void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string input; 
   for(auto i = begin(words) + 1; i != end(words) ; ++i){   
      input.append(*i); 
      input.append(" ");
   } 
state.change_prompt(input); 
}
//print the current working directory
void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
 
   int inode_nr= 0;
   inode_ptr ptr;
   ptr = state.get_cwd();
   inode_nr = ptr->get_inode_nr();
     if(inode_nr == 1){ 
    cout << "/" << endl;
    }else{ 
      state.get_cwd_string(inode_nr); 
    }

}
//deletes the specified file/directory
void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() == 1) cerr << "rm: No such file or directory "
                              <<"specified" << endl;
   inode_ptr ptr = state.get_cwd();
   string filename = words.at(1);
   ptr->remove(ptr,filename);
}
//recursively deletes files and directories until
//the specified file or directory is deleted
void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}
//forces shell to do nothing if first character is a #
void fn_cmt(inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);   
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

