#pragma once

//#include <cstdint>
#include <string_view>
#include <string>
//#include <deque>
#include <list>
#include <unordered_map>
#include <vector>

#include "geo.h"

//#define ID_TYPE uint16_t

//���������, ����������� ���������
struct Stop {
	//ID_TYPE id;
	std::string_view name;
	Coordinates coordinates;
};

//���� ��������� 
enum class RouteType {
	LINEAR,
	CIRCLE
};

//���������, ����������� �������
struct Route {
	//ID_TYPE id;
	std::string_view name;
	RouteType type;
	std::vector<const Stop*> stops;
};

//�������������� ���������� � ��������
struct RouteInfo {
	int stopsNumber;
	int uniqueStops;
	double length;
};

// ����� ������������� ����������� 
class TransportCatalogue {

public:

	TransportCatalogue() {
		//default constructor
	}

	// ���������� �������� � ����
	void addRoute(std::string& name, RouteType type, std::vector<std::string>& stops);

	//���������� ��������� � ����
	void addStop(std::string& name, Coordinates coordinates);

	//����� �������� �� �����
	const Route* findRoute(std::string_view name);

	//����� ��������� �� �����
	const Stop* findStop(std::string_view name);
	 
	//��������� ���������� � ��������
	//const RouteInfo getRouteInfo(const Route* route);

private:
	// ��������� ��� ���� ���� �����������. �� ��� ����� ��������� string_view ������.
	std::list<std::string> allNames;
	std::unordered_map<std::string_view, Stop*> nameToStop_;
	std::unordered_map<std::string_view, Route*> nameToRoute_;

	//������ ���� ���������
	std::list<Stop> stops_;

	//������ ���� ���������
	std::list<Route> routes_;

	std::string_view addString(std::string& str) {
		allNames.push_back(str);
		return allNames.back();
	}

};