// $Id: inode.h,v 1.13 2014-06-12 18:10:25-07 - - $
// Robert Ho rokho@ucsc.edu
// Daniel Urrutia deurruti@ucsc.edu

#ifndef __INODE_H__
#define __INODE_H__

#include <exception>
#include <iostream>
#include <memory>
#include <map>
#include <vector>
using namespace std;

#include "util.h"

//
// inode_t -
//    An inode is either a directory or a plain file.
//

enum inode_t {PLAIN_INODE, DIR_INODE};
class inode;
class file_base;
class plain_file;
class directory;

using inode_ptr = shared_ptr<inode>;
using file_base_ptr = shared_ptr<file_base>;
using plain_file_ptr = shared_ptr<plain_file>;
using directory_ptr = shared_ptr<directory>;

//
// inode_state -
//    A small convenient class to maintain the state of the simulated
//    process:  the root (/), the current directory (.), and the
//    prompt.
//

// whenever the root or current dir needs to be updated we need to 
// maintain the inode_state
// a friend class has access to the private functions of another 
// class in this case 
// the inode can access the next_inode_nr , inode_nr, inode_t type, 
// file_base_ptr contents 
class inode_state {
   friend class inode;
   friend ostream& operator<< (ostream& out, const inode_state&);
   private:
      inode_state (const inode_state&) = delete; //disable copy ctor
      //disable op=
      inode_state& operator= (const inode_state&) = delete;
      inode_ptr root {nullptr};
      inode_ptr cwd {nullptr};
      inode_ptr base{nullptr};
      string prompt {"% "};
   public:
      inode_state();
      void change_prompt(string& p); //set prompt 
      string get_prompt(); //get prompt
      inode_ptr get_root();
      inode_ptr get_cwd();
      void set_cwd(inode_ptr tmp);
      void set_root(inode_ptr tmp);
      void reset_ptr(); 
      void get_cwd_string(int num); 
      void set_base(); 
      inode_ptr get_base();
      
 }; 


//
// class inode -
//
// inode ctor -
//    Create a new inode of the given type.
// get_inode_nr -
//    Retrieves the serial number of the inode.  Inode numbers are
//    allocated in sequence by small integer.
// size -
//    Returns the size of an inode.  For a directory, this is the
//    number of dirents.  For a text file, the number of characters
//    when printed (the sum of the lengths of each word, plus the
//    number of words.
//    

class inode {
   friend class inode_state;
   private:
      static int next_inode_nr;
      int inode_nr;
      inode_t type;
      file_base_ptr contents;
   public:
      inode (inode_t init_type); // constructor 
      int get_inode_nr() const; //get inode nr
     // list command
      void ls(inode_ptr path, string& dirname,wordvec slash);
     
      file_base_ptr get_contents(); // get contents 
      void make(inode_ptr path, const wordvec& words);
      inode_t get_type(inode_ptr ptr);
      void find_file(inode_ptr path,string& file); 
      void remove(inode_ptr ptr,string& path);
      void mkdir(inode_ptr ptr, string& path);
      void find_dir(inode_state& state, string& dir);
      void reset_contents(inode_ptr path);
      // list command
      void lsr(inode_ptr path, string& dirname, wordvec& slash);  
      directory_ptr update_contents(inode_ptr path);
      plain_file_ptr update_pf(inode_ptr path);
};

//
// class file_base -
//
// Just a base class at which an inode can point.  No data or
// functions.  Makes the synthesized members useable only from
// the derived classes.
//
class file_base{
   protected:
      file_base () = default;
      file_base (const file_base&) = default;
      file_base (file_base&&) = default;
      file_base& operator= (const file_base&) = default;
      file_base& operator= (file_base&&) = default;
      virtual ~file_base () = default;
      virtual size_t size() const = 0;
   public:
      friend plain_file_ptr plain_file_ptr_of (file_base_ptr);
      friend directory_ptr directory_ptr_of (file_base_ptr);
};

//
// class plain_file -
//
// Used to hold data.
// synthesized default ctor -
//    Default vector<string> is a an empty vector.
// readfile -
//    Returns a copy of the contents of the wordvec in the file.
//    Throws an yshell_exn for a directory.
// writefile -
//    Replaces the contents of a file with new contents.
//    Throws an yshell_exn for a directory.
//

class plain_file: public file_base {
   private:
      wordvec data;
   public:
      size_t size() const override;
      const wordvec& readfile() const;        //get data
      void writefile (const wordvec& newdata);//set data       
};

//
// class directory -
//
// Used to map filenames onto inode pointers.
// default ctor -
//    Creates a new map with keys "." and "..".
// remove -
//    Removes the file or subdirectory from the current inode.
//    Throws an yshell_exn if this is not a directory, the file
//    does not exist, or the subdirectory is not empty.
//    Here empty means the only entries are dot (.) and dotdot (..).
// mkdir -
//    Creates a new directory under the current directory and 
//    immediately adds the directories dot (.) and dotdot (..) to it.
//    Note that the parent (..) of / is / itself.  It is an error
//    if the entry already exists.
// mkfile -
//    Create a new empty text file with the given name.  Error if
//    a dirent with that name exists.

class directory: public file_base {
   private:
      map<string,inode_ptr> dirents;
   public:
      size_t size() const override; //set&get size
      void remove (const string& filename);
      inode& mkdir (const string& dirname);
      inode& mkfile (const string& filename); 
      void mkroot(inode_ptr cwd,inode_ptr root);
      void ls(inode_ptr path,string& dirname, wordvec slash);
      void find_file(string& file);
      void mkdirents(directory_ptr d,inode_ptr cwd, inode_ptr root);
      void find_dir(inode_state& state, string& dirname);
      void find_nr(int num); 
      void lsr(inode_ptr path, string& dirname, wordvec& slash);
};

#endif

