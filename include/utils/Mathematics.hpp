#pragma once
#include "core/DataTypes.hpp"
#include <Eigen/Dense>
#include <vector>
#include <cmath>

namespace radar_tracking {

/**
 * @brief Mathematical utilities for tracking algorithms
 */
class Mathematics {
public:
    // Constants
    static constexpr double PI = M_PI;
    static constexpr double DEG_TO_RAD = PI / 180.0;
    static constexpr double RAD_TO_DEG = 180.0 / PI;
    static constexpr double EARTH_RADIUS_M = 6371000.0;  // WGS84 approximation
    
    // Distance calculations
    static double euclideanDistance(const Point3D& p1, const Point3D& p2);
    static double mahalanobisDistance(const Eigen::VectorXd& x1, 
                                     const Eigen::VectorXd& x2, 
                                     const Eigen::MatrixXd& covariance);
    
    // Coordinate transformations
    static Point3D cartesianToSpherical(const Point3D& cartesian);
    static Point3D sphericalToCartesian(const Point3D& spherical);
    static Point3D wgs84ToCartesian(double latitude, double longitude, double altitude);
    static void cartesianToWgs84(const Point3D& cartesian, double& latitude, double& longitude, double& altitude);
    
    // Matrix operations
    static Eigen::MatrixXd pseudoInverse(const Eigen::MatrixXd& matrix, double tolerance = 1e-6);
    static bool isPositiveDefinite(const Eigen::MatrixXd& matrix);
    static Eigen::MatrixXd makePositiveDefinite(const Eigen::MatrixXd& matrix, double epsilon = 1e-12);
    
    // Statistical functions
    static double gaussianPdf(const Eigen::VectorXd& x, 
                             const Eigen::VectorXd& mean, 
                             const Eigen::MatrixXd& covariance);
    static double chiSquaredCdf(double x, int degrees_of_freedom);
    static double chiSquaredInverseCdf(double p, int degrees_of_freedom);
    
    // Angle utilities
    static double normalizeAngle(double angle);
    static double angleDifference(double angle1, double angle2);
    
    // Kalman filter utilities
    static Eigen::MatrixXd createTransitionMatrix(double dt, int dimensions = 3);
    static Eigen::MatrixXd createProcessNoiseMatrix(double dt, double noise_variance, int dimensions = 3);
    static Eigen::MatrixXd createMeasurementMatrix(int state_size, int measurement_size);
    
    // Clustering utilities
    static std::vector<std::vector<int>> dbscanClustering(
        const std::vector<Point3D>& points, 
        double epsilon, 
        int min_points);
    
    // Association utilities
    static std::vector<std::pair<int, int>> hungarianAssignment(const Eigen::MatrixXd& cost_matrix);
    static Eigen::MatrixXd calculateCostMatrix(
        const std::vector<Track>& tracks,
        const std::vector<Cluster>& clusters);
    
    // Interpolation
    static Point3D linearInterpolation(const Point3D& p1, const Point3D& p2, double t);
    static Point3D splineInterpolation(const std::vector<Point3D>& points, 
                                      const std::vector<double>& times, 
                                      double target_time);
    
    // Validation
    static bool isValidCovariance(const Eigen::MatrixXd& covariance);
    static bool isValidState(const Eigen::VectorXd& state);
    
private:
    // Helper functions for DBSCAN
    static std::vector<int> regionQuery(const std::vector<Point3D>& points, 
                                       int point_index, 
                                       double epsilon);
    static void expandCluster(const std::vector<Point3D>& points,
                             std::vector<int>& cluster_assignments,
                             int point_index,
                             int cluster_id,
                             double epsilon,
                             int min_points);
};

/**
 * @brief 3D rotation matrix utilities
 */
class RotationMatrix {
public:
    static Eigen::Matrix3d rotationX(double angle);
    static Eigen::Matrix3d rotationY(double angle);
    static Eigen::Matrix3d rotationZ(double angle);
    static Eigen::Matrix3d rotationFromEuler(double roll, double pitch, double yaw);
    static void eulerFromRotation(const Eigen::Matrix3d& rotation, 
                                 double& roll, double& pitch, double& yaw);
    static Eigen::Quaterniond quaternionFromRotation(const Eigen::Matrix3d& rotation);
    static Eigen::Matrix3d rotationFromQuaternion(const Eigen::Quaterniond& quaternion);
};

}  // namespace radar_tracking