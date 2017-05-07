//
// Created by iisus on 30.04.2017.
//
#include "writing.h"

using namespace std;

void write_data(Header& h, std::ifstream& in, std::ofstream& out){
    in.seekg(h.data_begin, ios::beg);
    out.write((char*)&h.data_len, sizeof(int32_t));
    for(int32_t i = 0; i < h.data_len; i++){
        int8_t c;
        in.read((char*)&c, sizeof(int8_t));
        out.write((char*)&c, sizeof(int8_t));
    }
}

void write_index_data(Index_data& idh, ifstream& in, ofstream& out){
    int32_t n = idh.data.size();
    n *= 12;
    out.write((char*)&n, sizeof(int32_t));
    n/=12;
    for(int32_t i = 0; i < n; i++){
        out.write((char*)&idh.data[i].first.first, sizeof(uint32_t));
        out.write((char*)&idh.data[i].first.second, sizeof(uint32_t));
        out.write((char*)&idh.data[i].second, sizeof(int32_t));
    }
}

void write_chunk_info_data(Chunk_info& cih, ifstream& in, ofstream& out){
    int32_t n = cih.data.size();
    n *= 8;
    out.write((char*)&n, sizeof(int32_t));
    n/=8;
    for(int32_t i = 0; i < cih.data.size(); i++){
        out.write((char*)&cih.data[i].first, sizeof(int32_t));
        out.write((char*)&cih.data[i].second, sizeof(int32_t));
    }
}

void writing_chunk(Chunk& ch, std::ifstream& in, std::ofstream& out, std::map<int32_t, Connection>& unique_connections){
    if(ch.compression == "bz2")
        ch.compression = "none";
    ifstream unarch(ch.unarch_file, ios::binary);
    out << ch;
    out.write((char*)&ch.data_len, sizeof(int32_t));
    int32_t pos = 0;
    for(int i = 0; i < ch.connections_info.size(); i++){
        if(ch.connections_to_write[ch.connections_info[i].conn]){
            out << unique_connections[ch.connections_info[i].conn];
            write_data(unique_connections[ch.connections_info[i].conn], in, out);
            pos += unique_connections[ch.connections_info[i].conn].all_size();
        }
        cout << "start writing " << ch.connections_info[i].data.size() << " messages" << endl;
        for(int j = 0; j < ch.connections_info[i].data.size(); j++){
            Message_header mh;
            if(ch.compression == "bz2"){
                unarch.seekg(ch.connections_info[i].data[j].second, ios::beg);
                unarch >> mh;
            }
            else{
                in.seekg(ch.data_begin + ch.connections_info[i].data[j].second, ios::beg);
                in >> mh;
            }
            out << mh;
            ch.connections_info[i].data[j].second = pos;
            pos += mh.all_size();
            //cout << mh;
            write_data(mh, in, out);
        }
        cout << "written all good message" << endl;
    }
    for(int i = 0; i < ch.connections_info.size(); i++){
        out << ch.connections_info[i];
        write_index_data(ch.connections_info[i], in, out);
    }
}

void writingBag(std::vector<Chunk> chunks, Bag_header bh, Config conf, std::ifstream& in, std::ofstream& out){
    out << bh;
    cout << bh;
    for(int i = 0; i < 4027; i++){
        char t = ' ';
        out.write(&t, 1);
    }
    int64_t size_of_file = 13 +  bh.all_size();
    for(int i = 0; i < chunks.size(); i++){
        cout << chunks[i];
        chunks[i].info.chunk_pos = size_of_file;
        size_of_file += chunks[i].all_size();
        writing_chunk(chunks[i], in, out, conf.unique_connections);

        //cout << "data_len = " << chunks[i].data_len << endl;
    }
    for(int i = 0; i < conf.unique_connections.size(); i++){
        if(conf.take[conf.unique_connections[i].conn]){
            out << conf.unique_connections[i].conn;
            write_data(conf.unique_connections[i], in, out);
        }
    }
    for(int i = 0; i < chunks.size(); i++){
        out << chunks[i].info;
        write_chunk_info_data(chunks[i].info, in, out);
    }
}
