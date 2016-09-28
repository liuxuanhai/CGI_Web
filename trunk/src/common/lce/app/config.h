#ifndef _CONFIG__IMPL_HPP_
#define _CONFIG__IMPL_HPP_

#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <vector>
#include <stdint.h>

using namespace std;

namespace lce
{
    namespace app
    {
        class LineReader;

        class Config
        {
        public:
            static const int MAX_SECTION_NUM = 100;

        public:
            struct Line
            {
                Line()
                {
                    changed = false;
                    deleted = false;
                }

                enum
                {
                    NORMAL_LINE = 0,
                    COMMENT_LINE = 1,
                    SECTION_LINE   = 2,
                    BLANK_LINE       = 3,
                };

                bool       changed;
                bool       deleted;
                uint32_t  no;
                uint32_t  type;
                string      key;
                string      value;
                string      comment;
            };

            struct Section
            {
                string  name;
                string  comment;
                vector<Line>      lines;
                map<string, uint32_t>    params;
            };

            Config();
            ~Config();

            int load(string cfg_file);
            void show();

            string get_string_param(const string& section, const string& param_name, const string& default_value="") const;
            int get_int_param(const string& section,const  string& param_name, int default_value=-1) const;
            double get_double_param(const string& section, const string& param_name, double default_value=0.0) const;
            int get_time_param(const string & section,const string & param_name,int default_value=0) const; //return seconds, sufix="s/m/h"
            unsigned long get_ulong_param(const string & section,const string & param_name,unsigned long default_value=0) const;    

        private:
            void reset();
            int do_load(fstream& in_stream);
            int parse_normal_line();
            int parse_section_line();
            int parse_comment_line();
            int parse_blank_line();

        private:
            string           _cfg_file;
            LineReader*      _lr;
            int32_t          _current_section;
            vector<Section>  _sections;
            int32_t          _section_num;
        };

        class LineReader
        {
        public:
            LineReader(fstream& in_stream)
                    :_inlimit (0),
                    _inoff(0),
                    _in_stream(in_stream)
            {

            }
            int readline();

            char _inbuf[8192];
            char _linebuf[10240];
            int   _linelimit;
            int   _inlimit;
            int   _inoff;

            fstream& _in_stream;
        };
    }
}

#endif

