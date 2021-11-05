#include <stdio.h>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <cctype>
#include <bits/stdc++.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

//helper function for comparing type pairs for sorting map
template <typename T1, typename T2>
struct less_second {
    typedef pair<T1, T2> type; //type pair for map
    bool operator ()(type const& a, type const& b) const { //compare the frequency of words to order
        return a.second < b.second;
    }
};

map<string, int> getFrequencyMap(vector<string> vec) {
    // Define an map
    map<string, int> M;
 
    // Traverse vector vec check if current element is present or not
    for (int i = 0; i < vec.size(); i++) {
 
        // If the current element is not found then insert current element with frequency 1
        if (M.find(vec[i]) == M.end()) {
            M[vec[i]] = 1;
        }
 
        // Else update the frequency
        else {
            M[vec[i]]++;
        }
    }
 
		return M;
}

vector<string> segmentString(string text) {
	vector<string> words{};
  size_t pos = 0;
  char c = ' '; //space character for getting words between
  string boundary(1, c);

  //check text and break off segments of words when it equals a boundary
  while ((pos = text.find(boundary)) != string::npos) { //iterate through text line
    words.push_back(text.substr(0, pos));
    text.erase(0, pos + boundary.length());
  }
	
	return words;
}

//checks each word in words vector to filter out noise words
vector<string> filterNoise(vector<string> words, vector<string> noise) {

	vector<string> filtered{}; //filtered words to be returned

	for (string word : words) { //iterate over input words
		int isNoise = 0;
		for (string pattern : noise) { //check through all noise words
			if (word == pattern || word == "" || word.size() < 2) {
				isNoise = 1;
				continue; //is noise word, jump out of loop
			};
		}
		if (isNoise == 0) filtered.push_back(word); //not noise word, add to filtered vec
	}
	return filtered;
}