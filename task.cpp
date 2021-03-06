#include "helpers.hpp"
#include <cstdlib>
#include <pthread.h>
#include <mutex>

#define NUM_THREADS 1

using namespace std;

mutex mtx; 

vector<string> line_queues[NUM_THREADS];
vector<string> noise{};
vector<string> total_words; //collective total of all words to be scanned

//data struct for passing data to thread
struct thread_data {
   int  thread_id; //unique identifier
   bool completed; //indicate thread finished execution
   bool allocation_finished; //indicate to thread the queue has finished allocating from main thread
   vector<string> *line_queue; //total lines to be processed by thread
   vector<string> *total_words; //total words to appending to
   vector<string> noise; //list of noise words to filter out
};

void *processLine(void *threadarg) {
   struct thread_data *my_data;
   my_data = (struct thread_data *) threadarg;
   
   while(true) {
      //get line from thread arg
      mtx.lock();

      if (my_data->line_queue->size() == 0 && my_data->allocation_finished) break;

      if (my_data->line_queue->size() > 0) {
        string line = my_data->line_queue->back();
        my_data->line_queue->pop_back();
        mtx.unlock();

        //line to lower case
        transform(line.begin(), line.end(), line.begin(), [](unsigned char c){ return tolower(c); });

        //replace non alphabetical characters with space
        std::replace_if(line.begin(), line.end(), [] (const char& c) { return !isalpha(c) ;},' ');

        //get segmented words from line and filter noise words
        vector<string> words = filterNoise(segmentString(line), my_data->noise);

        //append to all words vector
        mtx.lock();
        my_data->total_words->insert(end(*(my_data->total_words)), begin(words), end(words));
      }
      
      mtx.unlock();
   }

   my_data->completed = true;

   pthread_exit(NULL);
}

int main (int argc, char **argv) {

  // Get starting timepoint
  auto start = high_resolution_clock::now();

  struct dirent *dp;
  DIR *dfd;

  char *dir;
  dir = argv[1];

  //exit if did not provide search directory
  if (argc == 1) {
    printf("Usage: %s dirname\n", argv[0]);
    return 0;
  }

  //open directory
  if ((dfd = opendir(dir)) == NULL) {
    fprintf(stderr, "Can't open %s\n", dir);
    return 0;
  }

  //open noise txt file and populate noise vector
  ifstream noise_file("noise.txt");
  if (!noise_file) {/*error*/}
  string line;
  while (getline(noise_file, line)) { noise.push_back(line); }

  pthread_t threads[NUM_THREADS]; //thread objs
  struct thread_data td[NUM_THREADS];  //thread obj data

  //initialize thread queues
  for(int i = 0; i < NUM_THREADS; i++ )
    line_queues[i] = vector<string>();

  //create threads first, they'll be active to recieve lines from the main thread, hence multi tasks
  //soley task parallelism since there's only one thread in this example
  for(int i = 0; i < NUM_THREADS; i++ ) {
    cout <<"main() : creating thread, " << i << endl;

    td[i].thread_id = i;
    td[i].line_queue = &(line_queues[i]);
    td[i].total_words = &total_words;
    td[i].noise = noise;
    td[i].completed = false;
    td[i].allocation_finished = false;
    
    if (pthread_create(&threads[i], NULL, processLine, &td[i])) {
        cout << "Error:unable to create thread, " << i << endl;
        exit(-1);
    }
  }

  //loop through all files and give lines to line processing thread
  int iter = 0;
  while (dp = readdir(dfd)) {
    printf ("%s, %d\n", dp->d_name, dp->d_type);
    if (dp->d_type == 8) { //check that iterated pointer is file

        //vars for directory, file name, and line string
        string dir_str(dir);
        string f_name(dp->d_name);
        string line;

        ifstream infile(dir_str + "/" + f_name); //open current file for reading

        //iterate each line of file
        while (getline(infile, line)) {
          int thread_id = (iter++) % NUM_THREADS;
          mtx.lock();
          line_queues[thread_id].push_back(line); //assign line for processing to threads
          mtx.unlock();
        }
    }
  }

  //signal threads allocating lines is done
  for (int i = 0; i < NUM_THREADS; i++) {
    td[i].allocation_finished = true;
  }

  //wait for threads to complete
  while (true) {
    bool done = true;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (td[i].completed == false) {
          done = false;
          break;
        } 
    }
    if (done) break;
  }

  //map each word to its occurance frequency
  map<string, int> M;
  // Traverse vector vec check if current element is present or not
  for (int i = 0; i < total_words.size(); i++) {
    // If the current element is not found then insert current element with frequency 1
    if (M.find(total_words[i]) == M.end()) {
        M[total_words[i]] = 1;
    }
    // Else update the frequency
    else {
        M[total_words[i]]++;
    }
  }

  //dump word map a vector for sorting by frequency
  vector<pair<string, int> > mapcopy(M.begin(), M.end());
  sort(mapcopy.begin(), mapcopy.end(), less_second<string, int>()); //do sort

  //create csv output file and write header
  ofstream output("task.csv");
  output << "word" << ", " << "freq" << endl;

  //iterate through top 10% occuring words and write to csv
  for (int i = mapcopy.size() - 1; i > (int)(mapcopy.size() * .9); i--) {
    output << mapcopy[i].first << ", " << mapcopy[i].second << endl;
  }

  //close out of file
  output.close();

  //close directory
  closedir(dfd);

  // Get ending timepoint
  auto stop = high_resolution_clock::now();

  // Get duration
  auto duration = duration_cast<microseconds>(stop - start);

  cout << "Time taken by program: " << duration.count() << " microseconds" << endl;

  pthread_exit(NULL);
}