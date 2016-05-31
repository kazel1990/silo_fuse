#include "libsilo.h"

bool do_chunking(std::vector<char> in, std::vector<std::vector<char>>& out)
{
    unsigned int window = 0;
    if(!out.empty()) return false;
    std::vector<char>::iterator it_l = in.begin(), it_r = in.begin();
    while(it_r != in.end())
    {
        window <<= 8;
        window += (unsigned char) (*it_r) + 17;
        it_r++;
        if(!(window%CHUNK_MOD))
        {
            std::vector<char> chunk;
            while(it_l != it_r)
            {
                chunk.push_back(*it_l);
                it_l++;
            }
            out.push_back(chunk);
            window = 0;
        }
    }
    if(it_l != in.end())
    {
        std::vector<char> chunk;
        while(it_l != in.end())
        {
            chunk.push_back(*it_l);
            it_l++;
        }
        out.push_back(chunk);
    }
    return true;
}
