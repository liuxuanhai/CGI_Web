#include <iostream>
#include <fstream>
#include <sstream>
#include "config.h"

namespace lce
{
    namespace app
    {
        Config::Config()
        {
            reset();
        }

        Config::~Config()
        {

        }

        int Config::load(string cfg_file)
        {
            int rv = 0;

            reset();
            _cfg_file = cfg_file;

            fstream conf;

            conf.open(_cfg_file.c_str(), ios::in | ios::binary);
            if(!conf)
            {
                conf.close();
                return -1;
            }

            rv = do_load(conf);
            conf.close();

            return rv ;
        }

        int  Config::do_load(fstream& in_stream)
        {
            int rv = 0;
            char c = 0;
            int limit = 0;

            _lr = new LineReader(in_stream);

            while (((limit = _lr->readline()) >= 0)
                    &&(rv == 0))
            {
                c = _lr->_linebuf[0];
                switch(c)
                {
                case '[':
                    rv = parse_section_line();
                    break;

                case '#':
                case '!':
                    rv =parse_comment_line();
                    break;

                default:
                    rv = parse_normal_line();
                    break;
                }
            }

            delete _lr;
            _lr = NULL;

            return rv;
        }

        void Config::reset()
        {
            _cfg_file = "";
            _lr = NULL;
            _current_section=-1;
            _sections.clear();
            _sections.resize(MAX_SECTION_NUM);
            _section_num = 0;  
        }

        void Config::show()
        {
            for(unsigned i=0;i<_sections.size();i++)
            {
                cout<<"["<<_sections[i].name<<"]:"<<endl;
                for(map<string,uint32_t>::iterator it=_sections[i].params.begin(); it!=_sections[i].params.end(); ++it)
                {
                    cout<<"    "<<it->first<<endl;
                }
            }
        }

        string Config::get_string_param(const string& section, const string& param_name, const string& default_value) const
        {
            string section_name = "Foolish";
            if(section != "")
            {
                section_name = section;
            }

            for(int i=0; i< _section_num; i++)
            {
                if(_sections[i].name == section_name)
                {
                    map<string, uint32_t> ::const_iterator iter;
                    iter =_sections[i].params.find(param_name);

                    if(iter == _sections[i].params.end())
                    {
                        return default_value;
                    }
                    else
                    {
                        return _sections[i].lines[iter->second].value;
                    }
                }
            }

            return default_value;
        }

        int Config::get_int_param(const string& section,const  string& param_name, int default_value) const
        {
            string tmp = get_string_param(section, param_name, "");
            if(tmp.empty())
            {
                return default_value;
            }

            return strtol(tmp.c_str(),0,0);
        }

        double Config::get_double_param(const string& section, const string&  param_name, double default_value) const
        {
            string tmp = get_string_param(section, param_name, "");
            if(tmp.empty())
            {
                return default_value;
            }

            return atof(tmp.c_str());
        }

        int Config::get_time_param(const string& section, const string& param_name,int default_value) const
        {
            string v = get_string_param(section, param_name, "");
            if(v.empty())
            {
                return default_value;
            }

            int second = -1;
            char s[256];
            strcpy(s,v.c_str());
            if((s[v.size()-1]=='s') || (s[v.size()-1]=='S'))
            {
                s[v.size()-1]='\0';
                second=atoi(s);
            }
            else if((s[v.size()-1]=='m') || (s[v.size()-1]=='M'))
            {       
                s[v.size()-1]='\0';
                second=atoi(s)*60;
            }
            else if((s[v.size()-1]=='h') || (s[v.size()-1]=='H'))
            {       
                s[v.size()-1]='\0';
                second=atoi(s)*3600;
            }
            else
            {
                return default_value;
            }

            if(second < 0)
            {
                return default_value;
            }

            return second;
        }
            
        unsigned long Config::get_ulong_param(const string & section,const string & param_name,unsigned long default_value) const
        {
            string v = get_string_param(section, param_name, "");
            if(v.empty())
            {
                return default_value;
            }

            return  strtoul(v.c_str(),NULL,10);
        }

        int Config::parse_normal_line()
        {
            int limit = _lr->_linelimit;
            int key_len;
            int value_start;
            char c;
            bool has_sep;
            bool preceding_backslash;


            c = 0;
            key_len = 0;
            value_start = limit;
            has_sep = false;

            preceding_backslash = false;
            while (key_len < limit)
            {
                c = _lr->_linebuf[key_len];
                //need check if escaped.
                if ((c == '=' ||  c == ':') && !preceding_backslash)
                {
                    value_start = key_len + 1;
                    has_sep = true;
                    break;
                }
                else if ((c == ' ' || c == '\t' ||  c == '\f') && !preceding_backslash)
                {
                    value_start = key_len + 1;
                    break;
                }
                if (c == '\\')
                {
                    preceding_backslash = !preceding_backslash;
                }
                else
                {
                    preceding_backslash = false;
                }
                key_len++;
            }

            while (value_start < limit)
            {
                c = _lr->_linebuf[value_start];
                if (c != ' ' && c != '\t' &&  c != '\f')
                {
                    if (!has_sep && (c == '=' ||  c == ':'))
                    {
                        has_sep = true;
                    }
                    else
                    {
                        break;
                    }
                }
                value_start++;
            }

            string key(&_lr->_linebuf[0], key_len);
            string value;
            if(limit - value_start > 0)
            {
                value = string(&_lr->_linebuf[value_start], limit - value_start);
            }
            else
            {
                value = string("");
            }

            Line line;
            line.type = Line::NORMAL_LINE;
            line.key  = key;
            line.value  = value;

            if(_current_section == -1)
            {
                _section_num++;
                _current_section = 0;
                _sections[_current_section].name="globalsection";
                _sections[_current_section].lines.push_back(line);
                line.no =  _sections[_current_section].lines.size()-1;
                _sections[_current_section].params.insert(map<string, uint32_t>::value_type(key, line.no));
            }
            else
            {
                _sections[_current_section].lines.push_back(line);
                line.no =  _sections[_current_section].lines.size()-1;
                _sections[_current_section].params.insert(map<string, uint32_t>::value_type(key, line.no));

            }

            return 0;
        }

        int Config::parse_section_line()
        {
            int section_start = 0, section_end=0, tmp_section_end=0;
            int comment_start = 0;
            int iterator = 0;
            char c;
            bool preceding_backslash;
            bool error_ocurred = false;

            c = 0;
            preceding_backslash = false;

            for (iterator = 0; iterator < _lr->_linelimit; iterator++)
            {
                c = _lr->_linebuf[iterator];

                if(c=='[')
                {
                    if (iterator == 0)
                    {
                        continue;
                    }
                    else
                    {
                        error_ocurred  = true;
                        break;
                    }
                }

                if ((c == ' ' || c == '\t' ||  c == '\f'))
                {
                    if(section_start == 0)
                    {
                        continue;
                    }
                    else if(tmp_section_end == 0)
                    {
                        tmp_section_end = iterator -1;
                        continue;
                    }
                    else
                    {

                        continue;
                    }
                }


                if(c == ']')
                {
                    if(section_start > 0)
                    {
                        if(tmp_section_end==0)
                        {
                            section_end = iterator -1;
                        }
                        else
                        {
                            section_end = tmp_section_end;
                        }

                        continue;
                    }
                    else
                    {
                        error_ocurred  = true;
                        break;
                    }
                }

                if((c=='#') || (c=='!'))
                {
                    if((section_end > 0) && (section_start > 0))
                    {
                        comment_start = iterator;
                    }
                    else
                    {
                        error_ocurred = true;
                    }

                    break;
                }

                if (c == '\\')
                {
                    preceding_backslash = !preceding_backslash;
                }
                else
                {
                    preceding_backslash = false;
                }


                if(section_start == 0)
                {
                    section_start = iterator;
                }
                else if(tmp_section_end !=0)
                {
                    error_ocurred = true;
                    break;
                }

            }

            if(!error_ocurred)
            {
                if((section_start > 0) && (section_end > 0))
                {

                    string name(&_lr->_linebuf[section_start], section_end - section_start +1);

                    if(_current_section == -1)
                    {
                        _current_section = 0;
                    }
                    else
                    {
                        _current_section++;
                    }

                    _sections[_current_section].name = name;

                    if(comment_start != 0)
                    {
                        string comment(&_lr->_linebuf[comment_start], _lr->_linelimit - comment_start);
                        _sections[_current_section].comment = comment;
                    }

                    _section_num++;

                    return 0;

                }
                else
                {
                    return -1;
                }

            }
            else
            {
                return -1;
            }

        }

        int Config::parse_comment_line()
        {
            Line line;
            string comment_line(&_lr->_linebuf[0], _lr->_linelimit);

            line.type = Line::COMMENT_LINE;
            line.comment = comment_line;

            _sections[_current_section].lines.push_back(line);

            return 0;
        }

        int Config::parse_blank_line()
        {
            Line line;
            line.type = Line::BLANK_LINE;
            _sections[_current_section].lines.push_back(line);

            return 0;
        }

        int LineReader::readline()
        {
            char c = 0;

            bool skip_whitespace = true;
            bool is_commentline = false;
            bool is_newline = true;
            bool appended_linebegin = false;
            bool preceding_backslash = false;
            bool skip_LF = false;

            _linelimit = 0;

            while (true)
            {
                if (_inoff >= _inlimit)
                {
                    _in_stream.read(_inbuf, 8192);
                    _inlimit = _in_stream.gcount();
                    _inoff = 0;
                    if (_inlimit <= 0)
                    {
                        if (_linelimit == 0 || is_commentline)
                        {
                            return -1;
                        }
                        return _linelimit;
                    }
                }

                //The line below is equivalent to calling a
                //ISO8859-1 decoder.
                c = (char) (0xff & _inbuf[_inoff++]);
                if (skip_LF)
                {
                    skip_LF = false;
                    if (c == '\n')
                    {
                        continue;
                    }
                }

                if (skip_whitespace)
                {
                    if (c == ' ' || c == '\t' || c == '\f')
                    {
                        continue;
                    }
                    if (!appended_linebegin && (c == '\r' || c == '\n'))
                    {
                        continue;
                    }
                    skip_whitespace = false;
                    appended_linebegin = false;
                }
                if (is_newline)
                {
                    is_newline = false;
                    if (c == '#' || c == '!')
                    {
                        is_commentline = true;
                        continue;
                    }
                }

                if (c != '\n' && c != '\r')
                {
                    _linebuf[_linelimit++] = c;
                    if (_linelimit == 10240)
                    {
                        cout<<"Line too long in config file!"<<_linebuf<<endl;
                        return -1;
                    }
                    //flip the preceding backslash flag
                    if (c == '\\')
                    {
                        preceding_backslash = !preceding_backslash;
                    }
                    else
                    {
                        preceding_backslash = false;
                    }
                }
                else
                {
                    // reached EOL
                    if (is_commentline || _linelimit == 0)
                    {
                        is_commentline = false;
                        is_newline = true;
                        skip_whitespace = true;
                        _linelimit = 0;
                        continue;
                    }
                    if (_inoff >= _inlimit)
                    {
                        _in_stream.read(_inbuf, 8192);
                        _inlimit = _in_stream.gcount();
                        _inoff = 0;

                        if (_inlimit <= 0)
                        {
                            return _linelimit;
                        }
                    }
                    if (preceding_backslash)
                    {
                        _linelimit -= 1;
                        //skip the leading whitespace characters in following line
                        skip_whitespace = true;
                        appended_linebegin = true;
                        preceding_backslash = false;
                        if (c == '\r')
                        {
                            skip_LF = true;
                        }
                    }
                    else
                    {
                        return _linelimit;
                    }
                }
            }
        }
    }
}
