#include "search_server.h"
#include "iterator_range.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

// it would be better, if we use string_view
vector<string_view> SplitIntoWords(string_view line) 
{
    vector<string_view> result;
    size_t current = line.find_first_not_of(' ', 0);
    while (true) 
    {
        auto space = line.find(' ', current);
        result.emplace_back(line.substr(current, space - current));
        if (space == line.npos)
        {
            break;
        }
        else
        {
            current = line.find_first_not_of(' ', space);
        }
        if (current == line.npos)
        {
            break;
        }
    }
    return result;
}

/*vector<string> SplitIntoWords(const string& line) {
  istringstream words_input(line);
  return {istream_iterator<string>(words_input), istream_iterator<string>()};
}*/

/****************************************************************/
/*                 S E A R C H   S E R V E R                    */
/****************************************************************/

SearchServer::SearchServer(istream& document_input) 
{
    UpdateDocumentBase(document_input);
}

void SearchServer::UpdateDocumentBase(istream& document_input) 
{
    InvertedIndex new_index;
    for (string current_document; getline(document_input, current_document); ) 
    {
        new_index.Add(move(current_document));
    }
    index = move(new_index);
}

void SearchServer::AddQueriesStream
(istream& query_input, ostream& search_results_output) 
{
    // variables for our query
    vector<size_t> docs;
    vector<size_t> ind;
    // we should reserve memory, because using every single 
    // time push_back we waste a lot of time 
    docs.resize(50'000);
    ind.resize(50'000);
    //docs.resize(5);
    //ind.resize(5);

    //Step one: search query word by database
    for (string current_query; getline(query_input, current_query); ) 
    {
        size_t current_index = 0;
        for (const auto& word : SplitIntoWords(current_query)) 
        {
            // get information about query word 
            vector<Id_and_Count> vec = index.Lookup(word);
            for (const auto& [docid, count] : vec) 
            {
                if (docs[docid] == 0) 
                {
                    // where we meet this query word
                    ind[current_index++] = docid;
                }
                // how often we meet this query word
                docs[docid] += count;
            }
        } // for word

        // Step two: work with result search 
        vector<Id_and_Count> search_result;
        
        for (size_t docid = 0; docid < current_index; ++docid) 
        {
            size_t count = 0;
            size_t id = 0;
            // we need to grab the information from doc and id give them 0
            swap(count, docs[ind[docid]]);
            swap(id, ind[docid]);
            search_result.emplace_back(id, count);
        }

        // Step three: sorting the result 
        const size_t ANSWERS_COUNT = 5;
        // we shouldn't sort all 50'000 elements 
        partial_sort
        (   begin(search_result),
            begin(search_result) + min<size_t>(ANSWERS_COUNT, search_result.size()),
            end(search_result),
            [](Id_and_Count lhs, Id_and_Count rhs) 
            {
                int64_t lhs_docid = lhs.first;
                auto lhs_hit_count = lhs.second;
                int64_t rhs_docid = rhs.first;
                auto rhs_hit_count = rhs.second;
                return make_pair(lhs_hit_count, -lhs_docid) > make_pair(rhs_hit_count, -rhs_docid);
             }
        );

        // Step four: Output of results 
        search_results_output << current_query << ':';

        // why we should use Head() from "iterator_range.h" ?
        // Head saves us from working with the number of queries more than five
        for (auto [docid, hitcount] : Head(search_result,ANSWERS_COUNT)) 
        {
            search_results_output << " {"
                << "docid: " << docid << ", "
                << "hitcount: " << hitcount << '}';
        }
        search_results_output << endl;

    }// for current_query
}

/****************************************************************/
/*                I N V E R T E D   I N D E X                   */
/****************************************************************/

void InvertedIndex::Add(string&& document) 
{
    docs.push_back(move(document));
    const size_t docid = docs.size() - 1;
    for (const auto& word : SplitIntoWords(docs.back())) 
    {
        // with C++17 index[word] = create(word) in the map 
        /*Finaly we have:
        index[word] - create(word) - Id = NULL, Count = NULL */
        vector<Id_and_Count>& id_and_count = index[word];

        if (!id_and_count.empty() && id_and_count.back().first == docid)
        {
            id_and_count.back().second += 1;
        }
        else 
        {
            // when we meet a new word 
            // we not copy or move element (for push_back)
            // we create a new object
            id_and_count.emplace_back(docid, 1);
        }
    }
}

//void InvertedIndex::Add(const string& document) {
//    docs.push_back(document);
//
//    const size_t docid = docs.size() - 1;
//    for (const auto& word : SplitIntoWords(document)) {
//        index[word].push_back(docid);
//    }
//}

vector<pair<size_t, size_t>> InvertedIndex::Lookup(string_view word) const 
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
