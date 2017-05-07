//
// Created by iisus on 24.04.2017.
//

#include "config.h"

using namespace std;

int32_t get_index_of_id(vector<pair<int32_t, int32_t>>& data, int32_t id){
    for(int32_t i = 0; i < data.size(); i++){
        if(data[i].first == id){
            return i;
        }
    }
    return -1;
}

void comp_time(pair<uint32_t, uint32_t>& t, pair<uint32_t, uint32_t>& min, pair<uint32_t, uint32_t>& max){
    //if(t.first <)
    pair<uint32_t, uint32_t>f = std::min(t, min);
    min.first = f.first;
    min.second = f.second;
    pair<uint32_t, uint32_t>s = std::max(t, max);
    max.first = s.first;
    max.second = s.second;
}

void Config::parseChunk(Chunk& ch, ifstream& in){
    ifstream unarch(ch.unarch_file, ios::binary);
    //cout << endl << "NEXT"  << " " << ch.connections_info.size() << endl;
    ch.data_len = 0;
    for(int32_t i = 0; i < ch.connections_info.size(); i++){
        int32_t id = ch.connections_info[i].conn;
        //cout << id << "|";
        if(take[id] || !correct_conditions) {
            if (!taken[id]) {
                taken[id] = true;
                ch.connections_to_write[id] = true;
                ch.data_len += unique_connections[id].all_size();
            }
            //cout << "Parse messages from " << id << " connection" << endl;
            Index_data& idh = ch.connections_info[i];
            ch.info.start_time.first = UINT32_MAX;
            ch.info.start_time.second = UINT32_MAX;
            ch.info.end_time.first = 0;
            ch.info.end_time.second = 0;
            for(size_t j = 0; j < idh.data.size(); j++){
                if(checkTime(idh.data[j].first)^ (!correct_conditions) || (take[id] && !correct_conditions)){
                    comp_time(idh.data[j].first, start_time, end_time);
                    if(ch.compression == "bz2"){
                        unarch.seekg(idh.data[j].second, ios::beg);
                        ch.data_len += skip_header(unarch);
                    }
                    else{
                        in.seekg(idh.data[j].second + ch.data_begin, ios::beg);
                        ch.data_len += skip_header(in);
                    }
                }
                else{
                    idh.data.erase(idh.data.begin() + j--);
                    idh.count--;
                    idh.data_len -= 12;
                    ch.info.data[get_index_of_id(ch.info.data, id)].second--;
                }
            }
        }
        else{
            ch.connections_info.erase(ch.connections_info.begin() + i--);
            ch.info.count--;
        }

    }
    for(size_t i = 0; i < ch.info.data.size(); i++){
        if(take[ch.info.data[i].first] || !correct_conditions)
            ch.info.data.erase(ch.info.data.begin() + i--);
    }
    ch.size = ch.data_len;
}

bool Config::checkTime(std::pair<uint32_t, uint32_t > time){
    return (time >= start_time) && (time <= end_time);
}

int64_t string_to_long_long(string s){
    int64_t res = 0;
    int64_t dec = 1;
    for(int i = s.size() - 1; i >= 0; i--){
        res += dec * (int64_t)(s[i] - '0');
        dec *= 10;
    }
    return res;
}

void Config::parseTime(std::string time){
    vector<string > times(4);
    int ind = 0;
    string str_start_time, str_end_time;
    for(int i = 1; i < time.size()-1; i++){
        if(time[i] <= '9' && time[i] >= '0'){
            times[ind] += time[i];
        }
        else{
            ind++;
        }
    }
    start_time.first = (uint32_t)string_to_long_long(times[0]);
    start_time.second = (uint32_t)string_to_long_long(times[1]);
    end_time.first = (uint32_t)string_to_long_long(times[2]);
    end_time.second = (uint32_t)string_to_long_long(times[3]);
}

Config::Config(int argc, char** argv){
    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "--time")){
            parseTime(argv[++i]);
        }
        else if(!strcmp(argv[i], "--topic")){
            topic_regexp = argv[++i];
        }
        else if(!strcmp(argv[i], "-f") || !strcmp(argv[i], "--file")){
            i++;
            if(i >= argc){
                cout << "No in_file" << endl;
            }
            in_filename = argv[i];
        }
        else if(!strcmp(argv[i], "-c") || !strcmp(argv[i], "--output_correct")){
            i++;
            if(i >= argc){
                cout << "No out_file" << endl;
            }
            out_filename_correct = argv[i];
        }
        else if(!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output_other")){
            i++;
            if(i >= argc){
                cout << "No out_file" << endl;
            }
            out_filename_other = argv[i];
        }
        else{
            cout << argv[i] << "Wrong Option" << endl;
        }
    }
}

bool Config::parseConnection(Connection& ch){
    regex topic_r(topic_regexp);
    return regex_match(ch.topic, topic_r);
}

int32_t Config::parseConnections(){
    int32_t conn_count = 0;
    for(auto i = unique_connections.begin(); i != unique_connections.end(); ++i){
        int32_t id = i->second.conn;
        taken[id] = false;
        take[id] = parseConnection(i->second) ^ (!correct_conditions);
        if(take[id]){
            //cout << id << " " << i->second.topic << endl;
            conn_count++;
        }
    }
    return conn_count;
}

void Config::parseBag(std::vector<Chunk>& chunks, Bag_header& bh, std::ifstream& in){
    bh.conn_count = parseConnections();
    int64_t chunks_size = 0;
    for(int i = 0; i < chunks.size(); i++){
        Chunk& ch = chunks[i];
        parseChunk(chunks[i], in);
        chunks_size += ch.all_size();
        for(int32_t j = 0; j < ch.connections_info.size(); j++){
            chunks_size += chunks[i].connections_info[j].all_size();
        }
    }
    bh.index_pos = 13 + chunks_size + bh.all_size();
}