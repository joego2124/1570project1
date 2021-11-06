#include "helpers.hpp"

using namespace std;
using namespace std::chrono;

vector<string> noise{};

int main(int argc, char **argv) {

  /**
   * set up file opening and variables
   * 
   */

  // Get starting timepoint
  auto start = high_resolution_clock::now();

  struct dirent *dp;
  DIR *dfd;

  char *dir;
  dir = argv[1];

	vector<string> total_words; //collective total of all words to be scanned

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

  /**
   * loop through all files and process individual lines
   * 
   */
	//iterate through directory
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
				//line to lower case
				transform(line.begin(), line.end(), line.begin(), [](unsigned char c){ return tolower(c); });
        
        //replace non alphabetical characters with space
        std::replace_if(line.begin(), line.end(), [] (const char& c) { return !isalpha(c) ;},' ');

        //get segmented words from line
				vector<string> words = filterNoise(segmentString(line), noise);

        //append to all words vector
				total_words.insert(end(total_words), begin(words), end(words));
      }
    }
  }

  /**
   * count word frequency and write out to csv file
   * 
   */

  //map each word to its occurance frequency
	map<string, int> M = getFrequencyMap(total_words);
  
  //dump word map a vector for sorting by frequency
  vector<pair<string, int> > mapcopy(M.begin(), M.end());
  sort(mapcopy.begin(), mapcopy.end(), less_second<string, int>()); //do sort

  //create csv output file and write header
  ofstream output("serial.csv");
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
}

// https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c
// https://stackoverflow.com/questions/1271064/how-do-i-loop-through-all-files-in-a-folder-using-c
// https://www.delftstack.com/howto/cpp/cpp-split-string-by-space/
// https://www.geeksforgeeks.org/program-to-find-frequency-of-each-element-in-a-vector-using-map-in-c/
// https://www.gormanalysis.com/blog/reading-and-writing-csv-files-with-cpp/