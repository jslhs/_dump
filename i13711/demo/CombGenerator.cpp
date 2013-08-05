#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <memory>
#include <ctime>

using namespace std;

#include "CombGenerator.h"

/*
CombGenerator::CombGenerator()
: _buf(NULL)
, _bufSize(kBufferSize)
, _charCount(0)
{
}
*/

CombGenerator::CombGenerator(const char *charset, int len)
:_handler(NULL) 
,_buf(NULL)
, _bufSize(kBufferSize)
, _charCount(0)
, _len(len)
, _count(0)
, _charset(charset)
{
	if(charset != NULL)
		_charCount = strlen(charset);
	_inbuf[len] = '\0';
}

CombGenerator::~CombGenerator()
{
	if(_buf != NULL){
		delete _buf;
		_buf = NULL;
	}
}

long long CombGenerator::run()
{
	if(_handler == NULL)return 0;
	_count = 0;
	if(_buf != NULL)
	{
		delete _buf;
		_buf = NULL;
	}
	_buf = new char[kBufferSize];
	_cur = 0;
	gen(_len);
	(*_handler)(_buf, _cur * (_len + 2));
	_cur = 0;
	return _count;
}

void CombGenerator::gen(int level)
{
	if(level == 0)
	{
		int i = 0;
		int size = (_cur + 1) * (_len + 2);
		if(size >= kBufferSize)
		{
			(*_handler)(_buf, _cur * (_len + 2));
			_cur = 0;
			//memset(_buf, kBufferSize, 0);
		}
		for(; i<_len; i++)
		{
			_buf[_cur * (_len + 2) + i] = _inbuf[i];
		}
		_buf[_cur * (_len + 2) + i] = '\r';
		_buf[_cur * (_len + 2) + i + 1] = '\n';
		_count++;
		_cur++;
		//cout << _inbuf << endl;
		return;
	}
	else
	{
		
		for(int i=0; i<_charCount; i++)
		{
			_inbuf[_len-level] = _charset[i];
			gen(level - 1);
		}
	}
	
}

class MyHandler: public CombGenerator::Handler
{
public:
	MyHandler(const char *filename);
	~MyHandler();
	virtual int operator()(const char *buf, int len);

private:
	FILE *_fp;	
};

MyHandler::MyHandler(const char *filename)
{
	_fp = fopen(filename, "wb");
}

MyHandler::~MyHandler()
{
	if(_fp != NULL)
		fclose(_fp);
}

int MyHandler::operator()(const char *buf, int len)
{
	//fwrite(buf, len, 1, _fp);
	return 0;
}

int main(int argc, char *argv[])
{
	MyHandler handler("test.txt");
	CombGenerator gen("0123456789abcdefghijklmnopqrstuvwxyz", argc > 1?atoi(argv[1]):0); //abcdefghijklmnopqrstuvwxyz
	gen.setBufferHandler(&handler);
	clock_t start = clock();
	long long count = gen.run();
	clock_t end = clock();
	double dur = (double)(end - start) / CLOCKS_PER_SEC;
	cout << "speed: " << (((double)(count) / dur) / (1000000.0)) << "M c/s" << endl;
	return 0;
}



