#include "search_server.h"

#include "parse.h"
#include "iterator_range.h"

#include <algorithm>
#include <future>
#include <numeric>


/****************************************************************/
/*                I N V E R T E D   I N D E X                   */
/****************************************************************/

InvertedIndex::InvertedIndex(istream& document_input) 
{
  for (string current_document; getline(document_input, current_document); ) 
  {
    docs.push_back(move(current_document));
    size_t docid = docs.size() - 1;
    for (string_view word : SplitIntoWordsView(docs.back())) 
    {
      auto& docids = index[word];
      if (!docids.empty() && docids.back().first == docid) 
      {
        ++docids.back().second;
      } 
      else 
      {
        docids.push_back({docid, 1});
      }
    }
  }// for current_document
}

const vector<Id_and_Count>& InvertedIndex::Lookup(string_view word) const 
{
  if (auto it = index.find(word); it != index.end()) 
  {
    return it->second;
  } 
  else 
  {
      return {};
  }
}

/****************************************************************/
/*             S U P P O R T   F U N C T I O N                  */
/****************************************************************/

void UpdateIndex(istream& document_input, Synchronized<InvertedIndex>& index) 
{
  InvertedIndex new_index(document_input);
  // Danger zone. Many threads can refer to function
  // index -> {value, lock_guard(m)}, 
  // ref_to_value = value = map<string_view, vector<Id_and_Count>>
  swap(index.GetAccess().ref_to_value, new_index);
}

void ProcessSearches
 (istream& query_input,
  ostream& search_results_output,
  Synchronized<InvertedIndex>& index_handle) 
{
  vector<size_t> docs;
  vector<size_t> ind;

  for (string current_query; getline(query_input, current_query); ) 
  {

    {
        //Access type 
      auto access = index_handle.GetAccess();

      // Unlike the single-threaded version, we have to at every call
      // to change the size of the docid_count and docids vectors to the index, because
      // between successive iterations of the loop, the index can be changed
      // by running the Update Index function in parallel. Accordingly , in the new
      // the database version may have a different number of documents.

      // deque<string>.size()
      size_t doc_count = access.ref_to_value.GetDocuments().size();
      docs.assign(doc_count, 0);
      ind.resize(doc_count);

      auto& another_index = access.ref_to_value;
      for (const auto& word : SplitIntoWordsView(current_query))
      {
        for (const auto& [docid, hit_count] : another_index.Lookup(word)) 
        {
          docs[docid] = docs[docid] + hit_count;
        }
      }
    }
    const size_t ANSWERS_COUNT = 5;

    iota(ind.begin(), ind.end(), 0);
    {
      partial_sort
      ( begin(ind),
        Head(ind, ANSWERS_COUNT).end(),
        end(ind),
        [&docs](int64_t lhs, int64_t rhs) 
        {
          return pair(docs[lhs], -lhs) > pair(docs[rhs], -rhs);
        }
      );
    }//iota

    search_results_output << current_query << ':';
    for (size_t docid : Head(ind, ANSWERS_COUNT))
    {
      const size_t hit_count = docs[docid];
      if (hit_count == 0) 
      {
        break;
      }

      search_results_output << " {"
          << "docid: " << docid << ", "
          << "hitcount: " << hit_count << '}';
    }
    search_results_output << '\n';
  }
}

/****************************************************************/
/*                S E A R C H   S E R V E R                     */
/****************************************************************/

void SearchServer::UpdateDocumentBase(istream& document_input) 
{
  async_tasks.push_back(async(UpdateIndex, ref(document_input), ref(index)));
}

void SearchServer::AddQueriesStream
(istream& query_input, ostream& search_results_output) 
{
  async_tasks.push_back
  (async(ProcessSearches, ref(query_input), 
      ref(search_results_output), ref(index)));
}