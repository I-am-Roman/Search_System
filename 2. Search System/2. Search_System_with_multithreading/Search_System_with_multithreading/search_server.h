#pragma once

#include "synchronized.h"

#include <istream>
#include <ostream>
#include <vector>
#include <string>
#include <string_view>
#include <queue>

#include <map>

#include <future>
using namespace std;

using Id_and_Count = pair<size_t, size_t>;

class InvertedIndex 
{
private:  
  map<string_view, vector<Id_and_Count>> index;
  deque<string> docs;

public:

  InvertedIndex() = default;
  explicit InvertedIndex(istream& document_input);

  const vector<Id_and_Count>& Lookup(string_view word) const;

  const deque<string>& GetDocuments() const 
  {
    return docs;
  }


};

class SearchServer 
{
private:
  Synchronized<InvertedIndex> index;
  vector<future<void>> async_tasks;

public:
  SearchServer() = default;
  explicit SearchServer(istream& document_input)
    : index(InvertedIndex(document_input))
  {
  }

  void UpdateDocumentBase(istream& document_input);
  void AddQueriesStream(istream& query_input, ostream& search_results_output);

};

