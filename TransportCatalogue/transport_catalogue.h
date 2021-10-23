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
	double curvature; //������������. ��������� ������������ ����� �������� � ���������� (��������������) �����
};

// ����� ������������� ����������� 
class TransportCatalogue {

public:

	// ���������� �������� � ����
	void addRoute(std::string_view name, std::vector<std::string_view>& stops);
	//TODO: add input as AddRouteQuery

	//���������� ��������� � ����
	void addStop(std::string_view name, Coordinates coordinates);

	//�������� ��������� ��� ���������. ���������� ����������� ����� ����� updateCoordinates;
	//����� ��� ����, ����� �������� ��������� �����, ����� ��� ����������� ������� ���.
	void addStop(std::string_view name);

	// �������� ���������� ���������. ������� ��� ����, ����� ��� ��������� ���������� � ����������� 
	// ��������� �������� ��.
	void updateStopCoordinates(std::string_view name, Coordinates coordinates);

	//����� �������� �� �����
	const Route* findRoute(std::string_view name);

	//����� ��������� �� �����
	const Stop* findStop(std::string_view name);

	// �������� ���� �� ����� ���������?
	bool hasStop(std::string_view name);

	//��������� ��������� ����� ����� �����������
	void addStopsDistance(const Stop* stopA, const Stop* stopB, double distance);

	//��������� ���������� � ��������
	const RouteInfo getRouteInfo(const Route* route);
	const RouteInfo getRouteInfo(const std::string_view routeName);

	//����� ��������� �� ��������� 
	size_t getRoutesNumOnStop(const Stop* stop);

	std::vector<const Route*> getRoutesOnStop(const Stop* stop);

	// �������� ���������� ����� �����������. ���������� �������� ���������� ����� �����������.
	//���� ���������� �� �������� ���������� -1. ;
	//���� ���������� �� � �� � �� ������, ���������� 0. ;
	double getRealStopsDistance(const Stop* stopA, const Stop* stopB);

private:

	//�� ������������ ����������. ����� ��� �������� ������� ��������� � ���������.
	const Coordinates voidCoordinates = { 200.0, 200.0 };

	// ��������� ��� ���� ���� �����������. �� ��� ����� ��������� string_view ������.
	std::list<std::string> allNames;
	std::unordered_map<std::string_view, Stop*> nameToStop_; // key - name of the stop, value - ptr to Stop
	std::unordered_map<std::string_view, Route*> nameToRoute_;

	//������ ���� ���������
	std::list<Stop> stops_;

	//������ ���� ���������
	std::list<Route> routes_;

	// �������. ��������� - ��������
	std::unordered_map<const Stop*, std::unordered_set<Route*>> stopToRoutes_;


	// ����� ���� ���������� �� ���������. ���� ������� ����� A + � � � + �.
	class TwoStopHasher {
	public:
		size_t operator()(const std::pair<const Stop*, const Stop*> stops) const {
			/*TODO: modify like here https ://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
			hash(a, b) = (a + b) * (a + b + 1 ) / 2 + b; */

			std::hash<const void*> phasher; // ����� ��� ���������.
			return phasher(stops.first) + phasher(stops.second);
		}
	};

	// �������. ���������� ����� �����������. ���������� �� � �� � ����� ���� �� ����� ���������� �� � �� �.
	// ����� ����� ��������� ���������� �� � �� �.
	std::unordered_map<std::pair<const Stop*, const Stop*>, double, TwoStopHasher> stopsDistance;

	std::string_view addString(std::string_view str) {
		allNames.push_back(static_cast<std::string>(str));
		return allNames.back();
	}

};