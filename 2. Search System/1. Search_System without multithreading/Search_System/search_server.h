#pragma once

#include <istream>
#include <iostream>
#include <ostream>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <deque>

using namespace std;

using Id_and_Count = pair<size_t, size_t>;

/*The reverse (or inverted) index is widely used in real search
engines. The idea is very simple. Let's say we have N documents, 
each of which contains no more than K words, as well
as a search query consisting of Q words. If each of the Q words is searched in each
document, then it will have the asymptotics O(QNK). If we build a reverse index,
that is, for each word we build a list of documents in which it occurs, then
the search query will be processed in O(QN). Thus, we achieve the best
asymptotics of search query processing.*/

// Our DataBase
class InvertedIndex 
{
private:
    // it would be better, if we use a string_view instead of string
    // vector push back more better than list 
  map<string_view, vector<Id_and_Count>> index;
  deque<string> docs;

  //map<string, list<size_t>> index;
  //vector<string> docs;

public:
  // Here we should use r_value and move function
  void Add(string&& document);
  vector<vector<Id_and_Count>> Lookup(string_view word) const;

  const string& GetDocument(size_t id) const 
  {
    return docs[id];
  }

  void Show()
  {
      cout << "INDEX" << endl;
      for (auto it_one : index)
      {
          cout << it_one.first << "\t";
          for (auto it_two : it_one.second)
          {
              cout << it_two.first <<" "<<it_two.second << endl;
          }
      }
      cout << "DOCS" << endl;
      for (auto it_one : docs)
      {
          cout << it_one << endl;
      }
  }

};

class SearchServer 
{
private:
  InvertedIndex index;
public:
  SearchServer() = default;
  explicit SearchServer(istream& document_input);
  void UpdateDocumentBase(istream& document_input);
  void AddQueriesStream(istream& query_input, ostream& search_results_output);
};
