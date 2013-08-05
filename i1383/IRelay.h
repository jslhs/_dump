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
		std::pair<IEndpoint, IEndpoint> wire(from, to);
		_wires.push_back(wire);
		return _wires.size() - 1; 
	}

	virtual void disconnect(int index) 
	{
		if(index < 0 || index >= _wires.size()) throw std::invalid_argument("index underflow");
		auto wire = _wires[index];
		auto it = std::find(_wires.begin(), _wires.end(), wire);
		_wires.erase(it);
	}

private:
	std::vector<std::pair<IEndpoint, IEndpoint>> _wires;
};