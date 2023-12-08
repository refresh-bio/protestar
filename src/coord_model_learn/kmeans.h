#pragma once

#include <vector>
#include <array>
#include <vector>
#include <random>

#include <iostream>
#include "../common/tetrahedron.h"

template <class T, int NDims>
using table_t = std::vector<std::array<T, NDims>>;

template <class T, int NDims>
class SquaredEuclidean
{
public:
	T operator()(const std::array<T, NDims>& p, const std::array<T, NDims>& q) {
		T sqr_dist = 0;
		for (int i = 0; i < NDims; ++i) {
			T delta = p[i] - q[i];
			sqr_dist += delta * delta;
		}

		return sqr_dist;
	}
};

template <class T, int NDims>
class Euclidean
{
public:
	T operator()(const std::array<T, NDims>& p, const std::array<T, NDims>& q) {
		T sqr_dist = 0;
		for (int i = 0; i < NDims; ++i) {
			T delta = p[i] - q[i];
			sqr_dist += delta * delta;
		}

		return sqrt(sqr_dist);
	}
};

template <class T, int NDims>
class TetrahedronPrediction
{
	T dist2(const double_coords_t& a, const double_coords_t& b)
	{
		return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z);
	}

public:
	T operator()(const std::array<T, NDims>& p, const std::array<T, NDims>& q) {
		tetrahedron_t tetr1(p[0], p[1], p[2], p[3], p[4], p[5]);
		tetrahedron_t tetr2(p[0], p[1], p[2], q[3], q[4], q[5]);

		auto pred1 = tetr1.get_q_both();
		auto pred2 = tetr2.get_q_both();

		auto best_dd2 = sqrt(std::min({dist2(pred1.first, pred2.first), dist2(pred1.first, pred2.second), dist2(pred1.second, pred2.first), dist2(pred1.second, pred2.second) }));

		if (best_dd2 <= sqrt(3))
			return 0;
		else
			return 3.0 * log2(best_dd2 / sqrt(3));
	}
};

template <class T, int NDims>
class Manhattan
{
public:
	T operator()(const std::array<T, NDims>& p, const std::array<T, NDims>& q) {
		T dist = 0;
		for (int i = 0; i < NDims; ++i) {
			T delta = abs(p[i] - q[i]);
			dist += delta;
		}

		return dist;
	}
};

template <class T, int NDims, int NLast>
class BitCost
{	
public:
	
	T operator()(const std::array<T, NDims>& p, const std::array<T, NDims>& q) {
		T dist = 0;
		
		// iterate over last 
		for (int d = NDims - NLast; d < NDims; ++d) {
//			T delta = std::abs(p[d] - q[d]) * 1000;
			T delta = std::abs(p[d] - q[d]);
			delta = std::floor(delta);
			if (delta > 0) {
				dist += std::log2(delta) + 2; // number of bits required to store a delta
			}
		}

		return dist;
	}
};

template <class T, int NDims, class DistanceMetric>
T kmeans(
	const table_t<T, NDims>&points,
	table_t<T, NDims>& centroids,
	std::vector<int>& assignments,
	int K,
	T wss_epsilon,
	int max_iters) {

	DistanceMetric dist;
	
	assignments.resize(points.size(), -1);
	centroids.assign(points.begin(), points.begin() + K);
	std::vector<std::array<T, NDims>> new_centroids(centroids.size(), std::array<T, NDims>());
	std::vector<int> num_assigned(K);
	
	int N = (int)points.size();
	T wss = std::numeric_limits<T>::max(); // within sum of square

	for (int iter = 0; iter < max_iters; ++iter) {
		// reset wss
		T wss_prev = wss;
		wss = 0;
		
		// reset new centroid positions and assignments
		for (int k = 0; k < K; ++k) {
			new_centroids[k].fill(0);
			num_assigned[k] = 0;
		}
		
		// iterate over data points
		for (int j = 0; j < N; ++j) {

			// assign point with its closest centroid
			int closest_k = -1;
			T min_dist = std::numeric_limits<T>::max();
			for (int k = 0; k < K; ++k) {
				T d = dist(points[j], centroids[k]);
				if (d < min_dist) {

					min_dist = d;
					closest_k = k;
				}
			}
		
			if (closest_k >= K || closest_k < 0)
				std::cerr << "Err: " << closest_k << " " << K << " " << j << " " << N << 
				points[j][0] << " " << points[j][1] << " " << points[j][2] << " " << points[j][3] << " " << points[j][4] << " " << points[j][5] << " " << std::endl;

			// accumulate data points in the updated centroid positions
			for (int d = 0; d < NDims; ++d) {
				new_centroids[closest_k][d] += points[j][d];
			}
			++num_assigned[closest_k];
			wss += min_dist;
			assignments[j] = closest_k;
		}

		// divide centroids by the number of assigned points
		for (int k = 0; k < K; ++k) {
	//		std::cout << "(";
			if(num_assigned[k])
				for (int d = 0; d < NDims; ++d) {
					new_centroids[k][d] /= num_assigned[k];
		//			std::cout << new_centroids[k][d] << ", ";
				}			
	//		std::cout << "), ";
		}
	//	std::cout << "wss=" << wss << std::endl;

		centroids.swap(new_centroids);
	
		if (wss_prev - wss < wss_epsilon) {
			break;
		}
	}

	// Sort centroids from the most popular
	vector<pair<int, int>> centr_occ;
	for (int i = 0; i < K; ++i)
		centr_occ.emplace_back(num_assigned[i], i);

	sort(centr_occ.begin(), centr_occ.end());
	reverse(centr_occ.begin(), centr_occ.end());

	vector<int> centroid_rev(K);

	for (int i = 0; i < K; ++i)
	{
		new_centroids[i] = centroids[centr_occ[i].second];
		centroid_rev[centr_occ[i].second] = i;
	}

	for (auto& x : assignments)
		x = centroid_rev[x];

	centroids.swap(new_centroids);
	
	return wss;
}
