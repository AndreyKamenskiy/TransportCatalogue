#pragma once

#include <string_view>
#include <string>
#include <list>
#include <unordered_map>
#include <unordered_set>
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

	// ���������� �������� � ����
	void addRoute(std::string_view name, std::vector<std::string_view>& stops);
	//TODO: add input as AddRouteQuery

	//���������� ��������� � ����
	void addStop(std::string_view name, Coordinates coordinates);

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

	std::string_view addString(std::string_view str) {
		allNames.push_back(static_cast<std::string>(str));
		return allNames.back();
	}

};