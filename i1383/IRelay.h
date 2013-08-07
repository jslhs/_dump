#pragma once
#include <WinSock2.h>

#include <functional>
#include <string>
#include <vector>
//using namespace std;

class IEndpoint
{
public:
	virtual std::string desc() const = 0;
	virtual int handle() const = 0;
	//virtual void 
};

class IRelay
{
public:
	virtual int connect(const IEndpoint &from, const IEndpoint &to) = 0;
	virtual void disconnect(int) = 0;
	//virtual void enumConnections(std::function<void(const IEndpoint &from, const IEndpoint &to)> callback) = 0;
};

class RelayBuilder;
class EndpointBuilder;

class TcpEndpoint
	: public IEndpoint
{
public:
	TcpEndpoint()
	{
	}

	~TcpEndpoint()
	{
	}

	virtual std::string desc() const { return "tcp socket"; }
	virtual int handle() const { return _sock;}

private:
	SOCKET _sock;
};

class TcpRelay
	: public IRelay
{
public:
	TcpRelay(int port);
	~TcpRelay();

public:
	enum Direction
	{
		Single, Bidirection
	};

	virtual int connect(const IEndpoint &from, const IEndpoint &to) 
	{ 
		return 0;
	}

	virtual void disconnect(int index) 
	{
	}

private:
	std::vector<std::pair<IEndpoint, IEndpoint>> _wires;
};