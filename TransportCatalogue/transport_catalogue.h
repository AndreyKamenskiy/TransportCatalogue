#pragma once

#include <string_view>
#include <string>
#include <list>
#include <unordered_map>
#include <vector>

#include "geo.h"


//���������, ����������� ���������
struct Stop {
	std::string_view name;
	Coordinates coordinates;
};

//���������, ����������� �������
struct Route {
	std::string_view name;
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
	void addRoute(std::string& name, std::vector<std::string>& stops);
	//TODO: add name as string_view 
	//TODO: add input as AddRouteQuery

	//���������� ��������� � ����
	void addStop(std::string& name, Coordinates coordinates);
	//TODO: addStop(Stop stop)

	//����� �������� �� �����
	const Route* findRoute(std::string_view name);

	//����� ��������� �� �����
	const Stop* findStop(std::string_view name);
	 
	//��������� ���������� � ��������
	const RouteInfo getRouteInfo(const Route* route);
	const RouteInfo getRouteInfo(const std::string_view routeName);

private:
	// ��������� ��� ���� ���� �����������. �� ��� ����� ��������� string_view ������.
	std::list<std::string> allNames;
	std::unordered_map<std::string_view, Stop*> nameToStop_; // key - name of the stop, value - ptr to Stop
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