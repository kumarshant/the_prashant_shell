/*
*/
#include<iostream>
#include<map>
#include<string>
#include<algorithm>
#include<vector>
#include<functional>
#include<filesystem>
#include<cstdlib>
#include<fstream>
#include<deque>

#ifdef _WIN32
#include<windows.h>
#endif

using namespace std;
using namespace std::filesystem;

map<string , function<int(vector<string>&s)>> mp;
deque<string> command_history;

void set_color(const string& color) {
    #ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (color == "green") {
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
        } else if (color == "blue") {
            SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
        } else if (color == "red") {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        }
    #else
        if (color == "green") {
            cout << "\033[32m";
        } else if (color == "blue") {
            cout << "\033[34m";
        } else if (color == "red") {
            cout << "\033[31m";
        }
    #endif
}

void reset_color() {
    #ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    #else
        cout << "\033[0m";
    #endif
}

// create file 
int create_file(vector<string>& s) {
  if (s.size() != 1) {
      set_color("red");
      cout << "Usage: touch <filename>" << endl;
      reset_color();
      return 1;
  }
  path file_path = s[0];

  try {
      ofstream file(file_path);
      if (file) {
          set_color("green");
          cout << "File created: " << file_path << endl;
          reset_color();
      } else {
          set_color("red");
          cout << "Failed to create the file: " << file_path << endl;
          reset_color();
      }
  } catch (const filesystem_error& e) {
      set_color("red");
      cout << "Error: " << e.what() << endl;
      reset_color();
      return 1;
  }
  return 0;
}

// find f
int find_(vector<string>& s) {
  if (s.size() != 2) {
      set_color("red");
      cout << "Usage: find <directory> <filename>" << endl;
      reset_color();
      return 1;
  }
  path dir = s[0];
  string filename = s[1];

  if (!exists(dir) || !is_directory(dir)) {
      set_color("red");
      cout << "Directory does not exist: " << dir << endl;
      reset_color();
      return 1;
  }

  bool found = false;

  // Iterate recursively through the directory
  try {
      for (const auto& entry : recursive_directory_iterator(dir)) {
          if (entry.path().filename() == filename) {
              set_color("green");
              cout << "Found: " << entry.path() << endl;
              reset_color();
              found = true;
          }
      }

      if (!found) {
          set_color("red");
          cout << "File not found: " << filename << endl;
          reset_color();
      }

  } catch (const filesystem_error& e) {
      set_color("red");
      cout << "Error: " << e.what() << endl;
      reset_color();
      return 1;
  }

  return 0;
}
//create link
int link_function(vector<string>& s) {
  if (s.size() != 2) {
      set_color("red");
      cout << "Usage: ln <existing_file> <link_name>" << endl;
      reset_color();
      return 1;
  }

  path existing_file = s[0];
  path link_name = s[1];

  // Check if the source file exists
  if (!exists(existing_file) || !is_regular_file(existing_file)) {
      set_color("red");
      cout << "The source file does not exist: " << existing_file << endl;
      reset_color();
      return 1;
  }

  try {
      // Create a hard link
      create_hard_link(existing_file, link_name);
      set_color("green");
      cout << "Hard link created: " << link_name << " -> " << existing_file << endl;
      reset_color();
  } catch (const filesystem_error& e) {
      set_color("red");
      cout << "Error: " << e.what() << endl;
      reset_color();
      return 1;
  }

  return 0;
}
//move
  int mv(vector<string>&s){
    if(s.size()!=2){
        set_color("red");
        cout<<"right now we support only deleting one file into the other !"<<endl;
        cout<<"try `mv souce_dir destination_dir"<<endl;
        reset_color();
        return 1;
      }
      else{
       path src=s[0];
       path dest=s[1];

       error_code err;
       path cano_src=weakly_canonical(src,err);
       if(err){
           set_color("red");
           cout<<"source cannonical error"<<err.message()<<endl;
           reset_color();
           return 1;
       }
       error_code err1;
       path cano_dest=weakly_canonical(dest,err1);
       if(err1){
           set_color("red");
           cout<<"destination cannonical error"<<err1.message()<<endl;
           reset_color();
           return 1;
       }

       if(!exists(cano_src) ){
           set_color("red");
           cout<<" souce does not exist  "<<endl;
           reset_color();
           return 1;
       }

       if (exists(cano_dest)) {
        if (is_directory(cano_dest)) {
            cano_dest /= cano_src.filename();
        } else {
            set_color("blue");
            cout << "Warning: Destination already exists as a file. Overwriting..." << endl;
            char proceed;
            cout<<"do you want to proceed(y/n) "<<endl;
            reset_color();
            cin>>proceed;
            if(proceed!='y'){
                return 0;
            }
        }
    }

       if (equivalent(cano_src, cano_dest)) {
        set_color("red");
        cout << "Source and destination are the same!" << endl;
        reset_color();
        return 1;
    }
       
           try{
               rename(cano_src,cano_dest);
               set_color("green");
               cout<<cano_src<<" moved to "<<cano_dest<<endl;
               reset_color();
               return 0;
           }
           catch (const filesystem_error& e) {
               set_color("red");
               cout << "Filesystem error: " << e.what() << endl;
               reset_color();
               if (is_regular_file(cano_src) && !equivalent(cano_src, cano_dest)) {
                try {
                    copy(cano_src, cano_dest, copy_options::overwrite_existing);
                    remove(cano_src);
                    set_color("green");
                    cout << "Moved (via copy and delete) " << cano_src << " to " << cano_dest << endl;
                    reset_color();
                    return 0;
                } catch (const filesystem_error& copy_e) {
                    set_color("red");
                    cout << "Failed to copy and delete: " << copy_e.what() << endl;
                    reset_color();
                    return 1;
                }
            }
               return 1;
           }
           catch (const exception& e) {
               set_color("red");
               cout << "Error: " << e.what() << endl;
               reset_color();
               return 1;
           }
           catch (...) {
               set_color("red");
               cout << "Unknown error occurred while copying." << endl;
               reset_color();
               return 1;
           }
           
       
      }
      return 0;
}

  //copy 
int cp(vector<string>&s){
  if(s.size()!=2){
    set_color("red");
    cout<<"right now we support only copying one file into the other !"<<endl;
    cout<<"try `cp souce_dir destination_dir"<<endl;
    reset_color();
    return 1;
  }
  else{
   path src=s[0];
   path dest=s[1];
   error_code err;
   path cano_src=weakly_canonical(src,err);
   if(err){
       set_color("red");
       cout<<"source cannonical error"<<err.message()<<endl;
       reset_color();
       return 1;
   }
   error_code err1;
   path cano_dest=weakly_canonical(dest,err1);
   if(err1){
       set_color("red");
       cout<<"destination cannonical error"<<err.message()<<endl;
       reset_color();
       return 1;
   }
   if(!exists(cano_src) || !exists(cano_dest) || !is_directory(cano_src)){
       set_color("red");
       cout<<"either souce or destination does not exist "<<endl;
       reset_color();
       return 1;
   }
   else{
       try{
           copy(cano_src,cano_dest,copy_options::overwrite_existing);
           set_color("green");
           cout<<cano_src<<" copied to "<<cano_dest<<endl;
           reset_color();
           return 0;
       }
       catch (const filesystem_error& e) {
           set_color("red");
           cout << "Filesystem error: " << e.what() << endl;
           reset_color();
           return 1;
       }
       catch (const exception& e) {
           set_color("red");
           cout << "Error: " << e.what() << endl;
           reset_color();
           return 1;
       }
       catch (...) {
           set_color("red");
           cout << "Unknown error occurred while copying." << endl;
           reset_color();
           return 1;
       }
       
   }
   
  }
  return 0;
}

    //change directory
int cd(vector<string>&s){
  if(s.size()!=1){
      set_color("red");
      cout<<"Unspecified Directory"<<endl;
      reset_color();
      return 1;
  }
  else{
      if(s[0]==".." || s[0]=="../"){
         path current=current_path();
         if(current.has_parent_path()){
          path parent= current.parent_path();
          current_path(parent);
          return 0;
         }
         
         set_color("green");
         cout<<current_path().string();
         reset_color();
         return 0;
      }
      else if(s[0]=="~"){
          #ifdef _WIN32
          const char* home = getenv("USERPROFILE");
  #else
          const char* home = getenv("HOME");
  #endif
          if (home) {
              path home_path = home;
              current_path(home_path);
              return 0;
          } else {
              set_color("red");
              cout << "Home directory not found!" << endl;
              reset_color();
              return 1;
          }
      }
      
      path p= s[0];
      error_code err;
      path canonical=weakly_canonical(p,err);
      if(err){
          set_color("red");
          cout<<"Error "<<err.message()<<endl;
          reset_color();
          return 1;
      }
      else{
          if (exists(canonical)) {
              current_path(canonical);
              set_color("green");
              cout << "Changed to: " << current_path().string() << endl;
              reset_color();
              return 0;
          }
          else {
            set_color("red");
            cout<<"No such path exist !"<<endl;
            reset_color();
            return 1;
          }
      }
  }
  return 0;
}

//remove file 
  int rem(vector<string>&s){
    if(s.size()==0){
      set_color("red");
      cout<<"path not specified"<<endl;
      reset_color();
    }
    else if(s.size()==1){
      error_code err;
     string raw_path= s[0];
     path p(raw_path);
     path canonical_path=weakly_canonical(p ,err);
     if (err) {
      set_color("red");
      cerr << "Warning: Could not canonicalize path: " << err.message() << endl;
      reset_color();
      return 1;
  } 
  if(!is_regular_file(canonical_path)){
 set_color("red");
 cout<<"path does not refer to a regular file"<<canonical_path<<endl;
 reset_color();
  }
  if(exists(canonical_path)){
    bool removed = remove(canonical_path,err);
    if (err) {
      set_color("red");
      cout << "Error removing file: " << err.message() << endl;
      reset_color();
       return 1;
     }
      if (removed) {
        set_color("green");
        cout << "The operation completed successfully" << endl;
        reset_color();
        } else {
        set_color("red");
        cout << "File was not removed (may not have existed)" << endl;
        reset_color();
        } 
      }
      else {
      set_color("red");
      cout<<"path does not exist"<<endl;
      reset_color();
      }    
  }
   return 0;
  }

  //print command
  int echo(vector<string>& s){
    set_color("blue");
    for(int i=0;i<s.size();i++){
        cout<<s[i]<<" ";
    }
    cout<<endl;
    reset_color();
    return 0;
  }
  //exit command
  int exit_cmd(vector<string>& s) {
    return -1; 
}
  //current working directory
   int pwd(vector<string> &s){
     set_color("green");
     cout<<current_path().string()<<endl;
     reset_color();
     return 0;
   }
   //list all the files 
   int ls(vector<string>&s){
     set_color("blue");
     for(const auto& it: directory_iterator(current_path())){
      cout<<it.path().filename().string()<<endl;
     }
     reset_color();
     return 0;
   }
   int history(vector<string>& s) {
    set_color("blue");
    for (size_t i = 0; i < command_history.size(); ++i) {
        cout << i + 1 << ": " << command_history[i] << endl;
    }
    reset_color();
    return 0;
}
   int help(vector<string>& s){
    set_color("blue");
    cout<<"AVAILABLE COMMANDS"<<endl;

    cout << "echo\t: Print text on the terminal" << endl;
    cout << "pwd\t: Print current working directory" << endl;
    cout << "ls\t: List directory content" << endl;
    cout << "rm\t: Remove a file or empty directory" << endl;
    cout << "cd\t: Change the directory (.. or ../ for parent dir, ~ for root dir)" << endl;
    cout << "mkdir\t: Create a new directory" << endl;
    cout << "cp\t: Copy a file or directory" << endl;
    cout << "mv\t: Move or rename a file or directory" << endl;
    cout << "touch\t: Create an empty file or update the timestamp of an existing file" << endl;
    cout << "find\t: Search for a file or directory recursively" << endl;
    cout << "ln\t: Create a hard link to an existing file" << endl;

    // Shell specific commands
    cout << "history\t: Show command history" << endl;
    cout << "help\t: Show available commands" << endl;
    cout << "exit\t: Exit the shell" << endl;

    reset_color();
    return 0;
   }
   //creating directory
   int mkdir(vector<string> &s){
    if(s.size()>1){
      set_color("red");
      cout<<"invalid directory name!"<<endl;
      cout<<"try one string"<<endl;
      reset_color();
      return 1;
    }
    path newDir = current_path()/s[0];
    if (!exists(newDir)) {
        create_directory(newDir);
        set_color("green");
        cout << "Directory created: " << newDir.string() << endl;
        reset_color();
    } else {
        set_color("red");
        cout << "Directory already exists: " << newDir.string() << endl;
        reset_color();
    }

    return 0;
   }

  vector<string> vector_str(string &str){
    vector<string> result;
    string word;
     for(int i=0;i<str.size();i++){
       if(str[i]==' '){
          if(!word.empty()) {
            result.push_back(word);
            word="";
           }
           else continue;
       }
      else if (int(str[i])== 34){
        continue;
      }
       else {
        word+= str[i];
       }
     }
     if(!word.empty()){
       result.push_back(word);
     }
     return result;   
  }

  int parse_str(string & s){
    vector<string>words= vector_str(s);
      if(words.size()==0) {
        set_color("red");
        cout<<"not enough tokens ";
        reset_color();
        return 1;
      };
    string command= words[0];
    vector<string> arguments;
    arguments.insert(arguments.end(),words.begin()+1,words.end());
    if(mp.find(command)==mp.end()){
        set_color("red");
        cout<<"invalid command "<<endl;
        reset_color();
        return 0;
    }
    else{
      return mp[command](arguments);
    }
    return 0;
  }

int main(){
  set_color("green");
  cout<<"Greetings from the_prashant_shell, crafted by Prashant!\nThis CLI empowers Windows users with Linux-inspired commands for seamless file exploration and management.\nEnter 'help' to discover all commands."<<endl;
  reset_color();
    mp["echo"]=echo;
    //filesystem
    mp["ls"]=ls; 
    mp["cd"]=cd;
    mp["pwd"]=pwd;
    mp["mkdir"]=mkdir;
    mp["rm"]=rem;
    mp["cp"]=cp;
    mp["mv"]=mv;
    mp["touch"]=create_file;
    mp["find"]=find_;
    mp["ln"]=link_function;
    //shell specific methods 
    mp["history"]=history;
    mp["help"]=help;
    mp["exit"]=exit_cmd;

    string str="";
    while(true){
        cout<<endl;
        set_color("blue");
        cout<<"> ";
        reset_color();
        //learning getline
        getline(cin ,str);
        if (!str.empty()) {
            command_history.push_back(str);
            if (command_history.size() > 50) {
                command_history.pop_front();
            }
        }
        int result= parse_str(str);
        if(result ==-1) break;
    }
    return 0;
}