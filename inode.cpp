// $Id: inode.cpp,v 1.12 2014-07-03 13:29:57-07 - - $
// Robert Ho rokho@ucsc.edu
// Daniel Urrutia deurruti@ucsc.edu

#include <iostream>
#include <stdexcept>
#include <string>
using namespace std;

#include "debug.h"
#include "inode.h"

//returns an the next inode nr being 1
int inode::next_inode_nr {1};

//a parametrized constructor that takes in a file type
//sets first inode nr and switches the file type 
inode::inode(inode_t init_type):
   inode_nr (next_inode_nr++), type (init_type)
{
   switch (type) {
      case PLAIN_INODE:
         contents = make_shared<plain_file>();
         break;
      case DIR_INODE:
         contents = make_shared<directory>();
         break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}
//get the inode number
int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

plain_file_ptr plain_file_ptr_of (file_base_ptr ptr) { 
   plain_file_ptr pfptr = dynamic_pointer_cast<plain_file> (ptr);
   if (pfptr == nullptr) throw invalid_argument ("plain_file_ptr_of");
   return pfptr;
}

directory_ptr directory_ptr_of (file_base_ptr ptr) {
   directory_ptr dirptr = dynamic_pointer_cast<directory> (ptr);
   if (dirptr == nullptr) throw invalid_argument ("directory_ptr_of");
   return dirptr;
}
//get the size of the plain files
size_t plain_file::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   wordvec data = this->data;
       //Adds the word count to the character count of each word
   for(size_t i= 0; i < data.size(); i++){
      size += data[i].size();
   }

   return size;
}
//get the data stored in the plain files
const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);   
   return data;
}
//use inodes to find a file
void inode::find_file(inode_ptr path,string& file){
   
   file_base_ptr fb = path->contents;
   directory_ptr dir = directory_ptr_of(fb);
   dir->find_file(file);
}
//use arguments from inode to actually find a file
void directory::find_file(string& file){
   map<string,inode_ptr>::iterator it; 
   for(it = dirents.begin(); it != dirents.end(); ++it){
      inode_ptr curr = it->second;
      inode_t node_type = curr->get_type(it->second);
      string name = (*it).first; 
      if(node_type == PLAIN_INODE && name == file){
         file_base_ptr fb = curr->get_contents();
         plain_file_ptr pf = plain_file_ptr_of(fb);
         wordvec dat = pf->readfile();
         cout << dat.at(0) << endl;
         return;
      }   
   }
   cerr << "cat: "<<file << ": No such file or directory"<< endl;   
}
//write to a plain file
void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   string name; 
   for(auto i = begin(words) + 1; i != end(words) ; ++i){   
      name.append(*i); 
      name.append(" ");
   } 
   size_t whitespace = name.find_last_not_of(" ");
   if(whitespace != string::npos){
      name.erase(whitespace+1);
   }
   this->data.push_back(name);
}
//create a plain file
void inode::make(inode_ptr path, const wordvec& words){

   file_base_ptr fb = path->contents; 
   directory_ptr dir = directory_ptr_of(fb); 
   string filename = words.at(0);
   inode& ref = dir->mkfile(filename);

   inode_ptr pfile = make_shared<inode>(ref);
   file_base_ptr fbase = pfile->contents;
   plain_file_ptr pfptr = plain_file_ptr_of(fbase);
   pfptr->writefile(words); 

} 
//create an inode to represent plain files
inode& directory::mkfile(const string& filename){
//inode pf(PLAIN_INODE);
//inode_ptr pfptr = make_shared<inode>(pf);

   inode_ptr pfptr(new inode(PLAIN_INODE));
   dirents.insert(pair<string,inode_ptr>(filename,pfptr)); 
   return *pfptr.get(); 
} 
//return the contents of an inode
file_base_ptr inode::get_contents(){
   return this->contents;
} 

//get the size of a directory
size_t directory::size() const {
   size_t size {0};
   size = dirents.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

// FIX remove must be resetting all shared pointers to our 
// object. We must follow the data path and set any shared 
// pointers to null
void inode::remove(inode_ptr ptr,string& path){
   file_base_ptr fb = ptr->contents;
   directory_ptr dir = directory_ptr_of(fb);
   dir->remove(path);
   
}  
//remove from a directory
void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
   map<string,inode_ptr>::iterator it; 
   int count =0; 
   for(it = dirents.begin(); it != dirents.end(); ++it){
      inode_ptr curr = it->second;
      inode_t node_type = curr->get_type(it->second);
      string name = (*it).first;
      if(name == filename && node_type == PLAIN_INODE){ 
         dirents.erase(name);
      }
      
      if(name == filename && node_type == DIR_INODE){
         file_base_ptr fbase = curr->get_contents();
         directory_ptr dir = directory_ptr_of(fbase);
         it= dir->dirents.begin();
         while(it != dir->dirents.end()){
            count++;
            it++;
         }
     
         if(count == 2){ 

            dirents.erase(name);
            count =0;
         }else{
            cerr << "rm: " << filename << ": is not empty" << endl;
         }
         return;  
      }  
   }
}
//the constructor of inode_states
inode_state::inode_state() {
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt << "\""); 
   inode start(DIR_INODE);
   this->root = make_shared<inode>(start);
   this->cwd = make_shared<inode>(start);
   this->base = make_shared<inode>(start);
   
   
   file_base_ptr fb = cwd->contents;
   directory_ptr dir = directory_ptr_of(fb);
   dir->mkroot(cwd,root); 
}
//returns the base state
inode_ptr inode_state::get_base(){
   return this->base;
}
//sets the location in the shell to the root of the shell
void inode_state::set_base(){
     this->root = this->get_base();
     this->cwd = this->get_base();
} 
//destroys the shared pointers
void inode_state::reset_ptr(){
   this->cwd->reset_contents(this->cwd);
   this->root->reset_contents(this->root);
   this->root.reset();
   this->cwd.reset();

}

void inode::reset_contents(inode_ptr path){
   path->contents.reset();
}
//make the root of the shell
void directory::mkroot(inode_ptr cwd, inode_ptr root){

   dirents.insert(pair<string, inode_ptr>(".",cwd));
   dirents.insert(pair<string, inode_ptr>("..",root));

}
//find a directory
void inode::find_dir(inode_state& state, string& dir){ 
   inode_ptr cwd = state.get_cwd();
   inode_ptr root = state.get_root();

   file_base_ptr fb = cwd->contents;
   directory_ptr direct = directory_ptr_of(fb);
   direct->find_dir(state, dir);
   //iterate over a map
   //find directory name
   //change cwd to directory if found
   //error if not found
} 
//sets the cwd
void inode_state::set_cwd(inode_ptr tmp){
   this->cwd = tmp;
}
//set the root
void inode_state::set_root(inode_ptr tmp){
   this->root = tmp;

}

void inode_state::get_cwd_string(int num){ 

   inode_ptr curr = this->root;
   file_base_ptr fb = curr->get_contents();
   directory_ptr dir = directory_ptr_of(fb);
   dir->find_nr(num);
} 

void directory::find_nr(int num){ 
   map<string,inode_ptr>::iterator it;
   for(it = dirents.begin(); it != dirents.end(); ++it){
      inode_ptr curr = it->second;
      int inode_nr = curr->get_inode_nr();
      if(inode_nr == num){ 
         cout << (*it).first << endl;
      }
   }
}

void directory::find_dir(inode_state& state, string& dirname){
   map<string,inode_ptr>::iterator it; 
   inode_ptr temp;
   for(it = dirents.begin(); it != dirents.end(); ++it){
      inode_ptr curr = it->second;      
      inode_t node_type = curr->get_type(it->second);
      string name = (*it).first;
 
      if(name == "."){
         temp = curr;         
      } 
      // if user calls cd or cd /
      if(dirname == "" || dirname == "/"){
         file_base_ptr fb = curr->get_contents();
         
         directory_ptr pf = directory_ptr_of(fb);
         state.set_root(temp);
         it= pf->dirents.begin();
         inode_ptr base = it->second;

         while(it != pf->dirents.end()){ 
            string dot = (*it).first;
            if(dot == ".."){
               state.set_cwd(base);
            } 
            it++;
         }
         return;        
      }
      if(node_type == DIR_INODE && name == dirname){
         file_base_ptr fb = curr->get_contents();
         
         directory_ptr pf = directory_ptr_of(fb);
         state.set_root(temp);
         it= pf->dirents.begin();
         inode_ptr next = it->second;

         while(it != pf->dirents.end()){ 
            string dot = (*it).first;
            if(dot == ".."){
               state.set_cwd(next);
            } 
            it++;
         }
         return;
      }
   
   }
   cerr << "cd: "<<dirname << ": No such directory"<< endl;   
}

void inode::ls(inode_ptr path, string& dirname, wordvec slash){
   file_base_ptr fb = path->contents;
   directory_ptr dir;
   dir->ls(path,dirname,slash);
} 

inode_t inode::get_type(inode_ptr ptr){
   return ptr->type;
}

void inode::lsr(inode_ptr path, string& dirname, wordvec& slash){ 
   file_base_ptr fb = path->contents;
   directory_ptr dir= directory_ptr_of(fb);
   dir->lsr(path,dirname,slash);     
   
} 
void directory::lsr(inode_ptr path, string& dirname, wordvec& slash){
   map<string,inode_ptr>::iterator it; 
   inode_t node_type;  
   
   string name = dirname;
   inode_ptr tmp = path;
   directory_ptr dirr = tmp->update_contents(tmp);
   plain_file_ptr pfile; 
   wordvec words = slash;
   wordvec directs;
   vector<inode_ptr> pointers;

   int wsize = words.size(); // >= 1 
   int i = 0  ;               // == wsize leave
   int isize = 0;
   int osize = 0;
   int onr =0;    
   // wsize == 0 if we just do ls 
   cout<<dirname<<":"<<endl;
   for(it = dirr->dirents.begin(); it != dirr->dirents.end(); ++it){
      inode_ptr curr = it->second;   //get current node 
      node_type = curr->get_type(it->second); // get the inode type 
      string f_name = (*it).first;         // get file_base name 
      if(node_type == DIR_INODE){
         file_base_ptr nb = curr->get_contents();
         directory_ptr dsize = directory_ptr_of(nb);
         isize = dsize->dirents.size();
      }
      //print any plain-file we don't care 
      if(node_type == PLAIN_INODE){
         file_base_ptr fb = curr->get_contents();
         pfile = plain_file_ptr_of(fb);
         cout << "      " << curr->get_inode_nr() << "      "  
              << pfile->size() <<"   "<<(*it).first << endl;
      }
      // determine which directory we need to print
      // case 1 we are at the directory we need to print 
      // OR words.size == 0 
      if(words.size() == 0 and node_type == DIR_INODE) { 
         if(f_name != "." and f_name != ".."){
            cout << "      " << curr->get_inode_nr() << "      " 
                << isize << "   " <<(*it).first <<"/"
                << endl;
            directs.push_back((*it).first);
            pointers.push_back(it->second);
         }
         else{
            cout << "      " << curr->get_inode_nr() << "      " 
                 << isize << "   " <<(*it).first << endl;
         }
      }
      //case 2 words.size >= 1 
      //first handle if we need to print ls .
      if(words.size() >= 1 and node_type == DIR_INODE){          
         name = words.at(i);
         //get the old inode number before the directory is updated
         if(wsize -1 == i+1){
            onr = curr->get_inode_nr();
            osize = dirr->dirents.size();
         } 
         if(f_name == name){ 
            dirr = curr->update_contents(curr);
            it = dirr->dirents.begin();       
         } 
         //cout << wsize << " "  << i << endl;      
 
         if(node_type == DIR_INODE){
           
            file_base_ptr nb = curr->get_contents();
            directory_ptr dsize = directory_ptr_of(nb);
            isize = dsize->dirents.size();
            if(f_name != "." and f_name != ".."){ 
               cout  <<"      "<<curr->get_inode_nr()<<"      "
                     <<dirr->dirents.size()<<"   "<<(*it).first <<"/"
                     << endl;
               directs.push_back((*it).first);
               pointers.push_back(it->second);
            }else if(f_name == "."){
               cout<<"      "<<curr->get_inode_nr()<<"      "
                   <<dirr->dirents.size()<<"   "<<(*it).first<<endl;
            }else{
               cout<<"      "<<curr->get_inode_nr()<<"      "
                   <<isize<< "   "<<(*it).first<<endl;
            }
         } 
         if(i == wsize and f_name == name){
            while(it != dirr->dirents.end()){               
               inode_ptr next = it->second;   //get current node 
               node_type = next->get_type(it->second);         
               string temp = (*it).first; 
               if(temp == ".."){ 
                  cout << "      " << onr << "      " 
                       << osize << "   " <<(*it).first 
                       << endl;             
               }else if(temp == "." ){ 
                  cout << "      " << curr->get_inode_nr() << "      " 
                       << dirr->dirents.size() << "   " <<(*it).first 
                       << endl;
               }else{                
                  if(node_type == PLAIN_INODE){
                     file_base_ptr nextfile = next->get_contents();
                     plain_file_ptr nf = plain_file_ptr_of(nextfile);
                     size_t pfsize = nf->size();
                     cout << "      " << next->get_inode_nr() 
                          << "      "  << pfsize <<"   "<<(*it).first 
                          << endl;
                  }                 
                  if(node_type == DIR_INODE){ 
                     file_base_ptr nextdir = next->get_contents();
                     directory_ptr nd = directory_ptr_of(nextdir);
                     size_t ndsize = nd->dirents.size();
                     directs.push_back((*it).first);
                     pointers.push_back(it->second);
                     cout << "      " << next->get_inode_nr() 
                          << "      " << ndsize <<"   "<<(*it).first
                          <<"/"<< endl;
                  }
               }             
               it++;
            }
            break;
         }        
         if(wsize != 1 and i < wsize-1 and f_name == name){ 
            i++;
         }         
      }           
   }
   size_t vsize = directs.size();
   if(vsize != 0){
      for(size_t iter =0; iter < vsize; iter++){
         lsr(pointers.at(iter),directs.at(iter),directs);
      }
   }
}
directory_ptr inode::update_contents(inode_ptr path){
   file_base_ptr fb = path->contents;
   directory_ptr dir= directory_ptr_of(fb); 

   return dir;
}
plain_file_ptr inode::update_pf(inode_ptr path){

   file_base_ptr fb = path->contents;
   plain_file_ptr pf = plain_file_ptr_of(fb);
   return pf;
}
void directory::ls(inode_ptr path,string& dirname,wordvec slash){ 
   map<string,inode_ptr>::iterator it; 
   inode_t node_type;  
   
   string name =dirname;
   inode_ptr tmp = path;
   directory_ptr dirr = tmp->update_contents(tmp);
   plain_file_ptr pfile; 
   wordvec words = slash;
 
   int wsize = words.size(); // >= 1 
   int i = 0  ;               // == wsize leave
   int isize = 0;
   int osize = 0;
   int onr =0;    
   cout<<dirname<<":"<<endl;
   // wsize == 0 if we just do ls 
   for(it = dirr->dirents.begin(); it != dirr->dirents.end(); ++it){
       inode_ptr curr = it->second;   //get current node 
       node_type = curr->get_type(it->second); // get the inode type 
       string f_name = (*it).first;         // get file_base name 
       if(node_type == DIR_INODE){
          file_base_ptr nb = curr->get_contents();
          directory_ptr dsize = directory_ptr_of(nb);
          isize = dsize->dirents.size();
       }
       //print any plain-file we don't care 
       if(node_type == PLAIN_INODE){
       file_base_ptr fb = curr->get_contents();
       pfile = plain_file_ptr_of(fb);
       cout << "      " << curr->get_inode_nr() << "      "  
            << pfile->size() <<"   "<<(*it).first << endl;
       }
       // determine which directory we need to print
       // case 1 we are at the directory we need to print 
       // OR words.size == 0 
       if(words.size() == 0 and node_type == DIR_INODE) { 
          if(f_name != "." and f_name != ".."){
             cout << "      " << curr->get_inode_nr() << "      " 
                  << isize << "   " <<(*it).first <<"/"
                  << endl;
          }
          else{
             cout << "      " << curr->get_inode_nr() << "      " 
                  << isize << "   " <<(*it).first << endl;
          }
       }
       //case 2 words.size >= 1 
       //first handle if we need to print ls .
       if(words.size() >= 1 and node_type == DIR_INODE){          
         name = words.at(i);
         //get the old inode number before the directory is updated
         if(wsize -1 == i+1){
            onr = curr->get_inode_nr();
            osize = dirr->dirents.size();
         } 
         if(f_name == name){ 
            dirr = curr->update_contents(curr);
            it = dirr->dirents.begin();       
         } 
         // cout << wsize << " "  << i << endl;      
 
         if(node_type == DIR_INODE){
            file_base_ptr nb = curr->get_contents();
            directory_ptr dsize = directory_ptr_of(nb);
            isize = dsize->dirents.size();
         } 
         if(i == wsize -1 and f_name == name){
            while(it != dirr->dirents.end()){               
               inode_ptr next = it->second;   //get current node 
               node_type = next->get_type(it->second);         
               string tmp = (*it).first; 
               if(tmp == ".."){ 
                  cout << "      " << onr << "      " 
                       << osize << "   " <<(*it).first 
                       << endl;             
               }else if(tmp == "." ){ 
                  cout << "      " << curr->get_inode_nr() << "      " 
                       << dirr->dirents.size() << "   " <<(*it).first 
                       << endl;
               }else{                
                  if(node_type == PLAIN_INODE){
                     file_base_ptr nextfile = next->get_contents();
                     plain_file_ptr nf = plain_file_ptr_of(nextfile);
                     size_t pfsize = nf->size();
                     cout << "      " << next->get_inode_nr() 
                          << "      " << pfsize <<"   "<<(*it).first
                          << endl;
                  }                 
                  if(node_type == DIR_INODE){ 
                     file_base_ptr nextdir = next->get_contents();
                     directory_ptr nd = directory_ptr_of(nextdir);
                     size_t ndsize = nd->dirents.size();
                     cout << "      " << next->get_inode_nr()
                          << "      " << ndsize <<"   "<<(*it).first 
                          << "/"<< endl;
                  }
               }             
               it++;
            }
            break;
         }        
         if(wsize != 1 and i < wsize-1 and f_name == name){ 
            i++;
         }         
      }           
   }
}

void inode::mkdir(inode_ptr ptr, string& path){
   file_base_ptr fb = ptr->contents;
   directory_ptr dir = directory_ptr_of(fb);
   string filename = path;
   //filename.append("/");
   inode& ref = dir->mkdir(filename);
   inode_ptr dirptr = make_shared<inode>(ref);
   file_base_ptr fbase = dirptr->contents;
   directory_ptr directs = directory_ptr_of(fbase);
   directs->mkdirents(directs,dirptr,ptr);
}

void directory::mkdirents(directory_ptr direct,inode_ptr cwd,
                           inode_ptr root){
   direct->dirents.insert(pair<string,inode_ptr>(".",cwd)); 
   direct->dirents.insert(pair<string,inode_ptr>("..",root));

}  

inode& directory::mkdir (const string& dirname){
   inode_ptr dptr(new inode(DIR_INODE));
   dirents.insert(pair<string,inode_ptr>(dirname,dptr)); 
   return *dptr.get(); 

}

//get root
inode_ptr inode_state::get_root(){ 
   return this->root;
} 

//get cwd
inode_ptr inode_state::get_cwd(){ 
   return this->cwd;
} 

//set prompt
void inode_state::change_prompt(string& input){ 
   prompt = input;
} 

//get prompt
string inode_state::get_prompt(){
   return prompt; 
} 

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

