/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 *
 */

#ifndef __ASIO2_INI_HPP__
#define __ASIO2_INI_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#	define NDEBUG
#endif

#include <cassert>
#include <cstring>

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include <asio2/util/def.hpp>
#include <asio2/util/helper.hpp>

namespace asio2
{

	/**
	 * standard ini file operator class 
	 * each time the function is invoked, it will re open the file and read the content.so 
	 * you call the function to get the value, you should save the value and save it later,
	 * when instead of calling the function to get the value every time.
	 */
	class ini
	{
	public:
		/*
		 * default constructor,the ini file path will be like this : 
		 * eg. if the executed application file is "D:/app/demo.exe",then the ini file is "D:/app/demo.ini"
		 */
		ini()
		{
#if   defined(LINUX)
			std::string dir(PATH_MAX, '\0');
			readlink("/proc/self/exe", (char *)dir.data(), PATH_MAX);
#elif defined(WINDOWS)
			std::string dir(MAX_PATH, '\0');
			dir.resize(::GetModuleFileNameA(NULL, (LPSTR)dir.data(), MAX_PATH));
#endif
			std::string filename;
			auto pos = dir.find_last_of(SLASH);
			if (pos != std::string::npos)
			{
				filename = dir.substr(pos + 1);
				dir.resize(pos + 1);
			}

			pos = filename.find_last_of('.');
			if (pos != std::string::npos)
			{
				filename.resize(pos);
			}

			filename += ".ini";

			m_filepath = dir + filename;
		}
		/*
		 * is_filename = true ,indicate the filename parameter is a file name string
		 * is_filename = false ,indicate the filename parameter is a completed file path string
		 */
		ini(const std::string & filename, bool is_filename = true)
		{
			if (is_filename)
				m_filepath = get_current_directory() + filename;
			else
				m_filepath = filename;
		}
		virtual ~ini()
		{
		}

		std::string get_filepath()
		{
			return this->m_filepath;
		}
		std::string set_filepath(const std::string & filepath)
		{
			auto old = std::move(this->m_filepath);
			this->m_filepath = filepath;
			return old;
		}

		bool get(const std::string & sec, const std::string & key, std::string & val)
		{
			std::fstream file(m_filepath, std::fstream::binary | std::fstream::in);
			if (file)
			{
				std::string line;
				std::string s, k, v;
				std::streampos posg;

				char ret;
				while ((ret = this->getline(file, line, s, k, v, posg)) != 'n')
				{
					switch (ret)
					{
					case 'a':break;
					case 's':
						if (s == sec)
						{
							while ((ret = this->getline(file, line, s, k, v, posg)) == 'k')
							{
								if (k == key)
								{
									val = v;
									return true;
								}
							}
							return false;
						}
						break;
					case 'k':
						if (sec == s)
						{
							if (k == key)
							{
								val = v;
								return true;
							}
						}
						break;
					case 'o':break;
					default:break;
					}
				}
			}
			return false;
		}

		bool set(const std::string & sec, const std::string & key, const std::string & val)
		{
			std::lock_guard<std::mutex> g(m_lock);
			std::fstream file(m_filepath, std::fstream::binary | std::fstream::in | std::fstream::out);
			if (file)
			{
				std::string line;
				std::string s, k, v;
				std::streampos posg = 0;

				char ret;
				while ((ret = this->getline(file, line, s, k, v, posg)) != 'n')
				{
					switch (ret)
					{
					case 'a':break;
					case 's':
						if (s == sec)
						{
							while ((ret = this->getline(file, line, s, k, v, posg)) == 'k')
							{
								if (k == key)
								{
									if (val == v)
										return true;

									std::string buffer;
									auto pos = line.find_first_of('=');
									pos++;
									while (pos < line.size() && std::isspace(line[pos]))
										pos++;
									buffer += line.substr(0, pos);
									buffer += val;
									buffer += LN;

									file.clear(); // must call this
									char c;
									while (file.get(c))
									{
										buffer += c;
									}

									file.clear(); // must call this
									file.seekp(posg);
									file.write(buffer.data(), buffer.size());

									return true;
								}
							}

							std::string buffer;
							buffer += key;
							buffer += '=';
							buffer += val;
							buffer += LN;

							file.clear(); // must call this
							file.seekg(posg);
							char c;
							while (file.get(c))
							{
								buffer += c;
							}

							file.clear(); // must call this
							file.seekp(posg);
							file.write(buffer.data(), buffer.size());

							return true;
						}
						break;
					case 'k':break;
					case 'o':break;
					default:break;
					}
				}

				std::string buffer;

				buffer += '[';
				buffer += sec;
				buffer += ']';
				buffer += LN;

				buffer += key;
				buffer += '=';
				buffer += val;
				buffer += LN;

				file.clear(); // must call this
				file.seekp(0, std::ios::end);
				file.write(buffer.data(), buffer.size());

				return true;
			}
			return false;
		}

		std::string get_string(const std::string & sec, const std::string & key, const std::string & default_val = std::string())
		{
			std::string val;
			if (!this->get(sec, key, val))
				val = default_val;
			return std::move(val);
		}

		int32_t get_int32(const std::string & sec, const std::string & key, int32_t default_val = 0)
		{
			std::string val;
			return (this->get(sec, key, val) ? std::atoi(val.data()) : default_val);
		}

		bool set_int32(const std::string & sec, const std::string & key, int32_t val)
		{
			return this->set(sec, key, format("%d", val));
		}

		uint32_t get_uint32(const std::string & sec, const std::string & key, uint32_t default_val = 0)
		{
			std::string val;
			return (this->get(sec, key, val) ? static_cast<uint32_t>(std::strtoul(val.data(), nullptr, 10)) : default_val);
		}

		bool set_uint32(const std::string & sec, const std::string & key, uint32_t val)
		{
			return this->set(sec, key, format("%u", val));
		}

		int64_t get_int64(const std::string & sec, const std::string & key, int64_t default_val = 0)
		{
			std::string val;
			return (this->get(sec, key, val) ? static_cast<int64_t>(std::strtoll(val.data(), nullptr, 10)) : default_val);
		}

		bool set_int64(const std::string & sec, const std::string & key, int64_t val)
		{
			return this->set(sec, key, format("%lld", val));
		}

		uint64_t get_uint64(const std::string & sec, const std::string & key, uint64_t default_val = 0)
		{
			std::string val;
			return (this->get(sec, key, val) ? static_cast<uint64_t>(std::strtoull(val.data(), nullptr, 10)) : default_val);
		}

		bool set_uint64(const std::string & sec, const std::string & key, uint64_t val)
		{
			return this->set(sec, key, format("%llu", val));
		}

		float get_float(const std::string & sec, const std::string & key, float default_val = 0.0f)
		{
			std::string val;
			return (this->get(sec, key, val) ? static_cast<float>(std::strtof(val.data(), nullptr)) : default_val);
		}

		bool set_float(const std::string & sec, const std::string & key, float val)
		{
			return this->set(sec, key, format("%f", val));
		}

		double get_double(const std::string & sec, const std::string & key, double default_val = 0.0)
		{
			std::string val;
			return (this->get(sec, key, val) ? static_cast<double>(std::strtod(val.data(), nullptr)) : default_val);
		}

		bool set_double(const std::string & sec, const std::string & key, double val)
		{
			return this->set(sec, key, format("%lf", val));
		}

	protected:
		char getline(std::fstream & file, std::string & line, std::string & sec, std::string & key, std::string & val, std::streampos & posg)
		{
			if (file.is_open())
			{
				posg = file.tellg();

				file.clear(); // must call this

				if (std::getline(file, line))
				{
					auto trim_line = line;

					trim_both(trim_line);

					// current line is code annotation
					if (
						(trim_line.size() > 0 && (trim_line[0] == ';' || trim_line[0] == ':'))
						||
						(trim_line.size() > 1 && trim_line[0] == '/' && trim_line[1] == '/')
						)
					{
						return 'a'; // annotation
					}

					auto pos1 = trim_line.find_first_of('[');
					auto pos2 = trim_line.find_first_of(']');

					// current line is section
					if (
						pos1 == 0
						&&
						pos2 != std::string::npos
						&&
						pos2 > pos1
						)
					{

						sec = trim_line.substr(pos1 + 1, pos2 - pos1 - 1);

						trim_both(sec);

						return 's'; // section
					}

					auto sep = trim_line.find_first_of('=');

					// current line is key and val
					if (sep != std::string::npos && sep > 0)
					{
						key = trim_line.substr(0, sep);
						trim_both(key);

						val = trim_line.substr(sep + 1);
						trim_both(val);

						return 'k'; // kv
					}

					return 'o'; // other
				}
			}

			return 'n'; // null
		}

	protected:
		std::string    m_filepath;

		std::mutex     m_lock;

	};
}

#endif // !__ASIO2_INI_HPP__
