#include "headers.h"
#include "config.h"
#include "writing.h"
using namespace std;

int main(int argc, char* argv[])
{
    Config conf(argc, argv);
    ifstream in(conf.in_filename, ios::binary);
    ofstream out(conf.out_filename_correct, ios::binary);
    for(int i = 0; i < 13; i++)
    {
        int8_t fuck;
        in.read((char*)&fuck, sizeof(int8_t));
        out.write((char*)&fuck, sizeof(int8_t));
        cout << fuck;
    }

    Bag_header bh;
    vector<Chunk> chunks;
    in >> bh;
    cout << bh;

    while(get_op(in) == 5)
    {
        Chunk ch;
        in >> ch;

        if(ch.compression == "bz2"){
            cout << "We have compressed chunk!!!" << endl;
            ch.unarch_file = "files_for_bag/chunk" + to_string(chunks.size());
            ofstream arch((ch.unarch_file + ".bz2"), ios::binary);
            write_data(ch, in, arch);
            //system("bunzip " + filename + ".bz2");TODO
        }
        ch.connections_to_write = conf.taken;
        in.seekg(ch.data_len, ios::cur);
        while(in && get_op(in) == 4)
        {
            Index_data idh;
            in >> idh;
            ch.connections_info.push_back(idh);
        }
        cout << "\n=================================\n" << endl;
        chunks.push_back(ch);
    }

    for(int32_t i = 0; i < bh.conn_count; i++)
    {
        Connection c;
        in >> c;
        cout << c;
        conf.unique_connections[c.conn] = c;
        in.seekg(c.data_len, ios::cur);
    }
    cout << "\n=================================\n" << endl;

    for(int32_t i = 0; i < bh.chunk_count; i++)
    {
        in >> chunks[i].info;
        cout << chunks[i].info;
    }
    cout << "\n=================================\nStart Writing" << endl;
    cout << "\n=================================\n" << endl;


    Bag_header bh_other = bh;
    vector<Chunk> chunks_other = chunks;

    conf.correct_conditions = true;
    conf.parseBag(chunks, bh, in);


    cout << "\n=================================\n" << endl;
    writingBag(chunks, bh, conf, in, out);
    cout << "\n=================================\n" << endl;
    conf.correct_conditions = false;
    conf.parseBag(chunks_other, bh_other, in);
    out = ofstream(conf.out_filename_other, ios::binary);
    writingBag(chunks_other, bh_other, conf, in, out);
    return 0;
}
