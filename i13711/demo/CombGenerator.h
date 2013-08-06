// Combination Generator

class CombGenerator
{
public:
	CombGenerator(const char *charset, int len = 1);
	~CombGenerator();

	class Handler
	{
	public:
		virtual int operator()(const char *buf, int len) = 0;
	};

public:
	void setBufferHandler(Handler *handler){_handler = handler;}
	long long run();
	static const int kBufferSize = (32 << 20);
	static const int kMaxLength = (32);

private:
	CombGenerator();
	void gen(int level);
	
	Handler *_handler;
	char *_buf;
	int _cur;
	int _bufSize;
	int _charCount;
	int _len;
	long long _count;
	const char *_charset;
	char _inbuf[kMaxLength];
};
