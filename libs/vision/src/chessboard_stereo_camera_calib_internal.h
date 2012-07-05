/* +---------------------------------------------------------------------------+
   |          The Mobile Robot Programming Toolkit (MRPT) C++ library          |
   |                                                                           |
   |                       http://www.mrpt.org/                                |
   |                                                                           |
   |   Copyright (C) 2005-2012  University of Malaga                           |
   |                                                                           |
   |    This software was written by the Machine Perception and Intelligent    |
   |      Robotics Lab, University of Malaga (Spain).                          |
   |    Contact: Jose-Luis Blanco  <jlblanco@ctima.uma.es>                     |
   |                                                                           |
   |  This file is part of the MRPT project.                                   |
   |                                                                           |
   |     MRPT is free software: you can redistribute it and/or modify          |
   |     it under the terms of the GNU General Public License as published by  |
   |     the Free Software Foundation, either version 3 of the License, or     |
   |     (at your option) any later version.                                   |
   |                                                                           |
   |   MRPT is distributed in the hope that it will be useful,                 |
   |     but WITHOUT ANY WARRANTY; without even the implied warranty of        |
   |     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
   |     GNU General Public License for more details.                          |
   |                                                                           |
   |     You should have received a copy of the GNU General Public License     |
   |     along with MRPT.  If not, see <http://www.gnu.org/licenses/>.         |
   |                                                                           |
   +---------------------------------------------------------------------------+ */

#ifndef MRPT_VISION_INTERNAL_STEREO_CALIB_H
#define MRPT_VISION_INTERNAL_STEREO_CALIB_H

#include <mrpt/vision/chessboard_camera_calib.h>
#include <mrpt/poses/CPose3D.h>
#include <mrpt/math/geometry.h>

namespace mrpt
{
	namespace vision
	{
		using namespace std;
		using namespace mrpt;
		using namespace mrpt::poses;
		using namespace mrpt::math;

		// State of the Lev-Marq optimization:
		struct lm_stat_t
		{
			const TCalibrationStereoImageList & images;
			const vector<size_t> & valid_image_pair_indices;
			const vector<TPoint3D> &obj_points;

			// State being optimized:
			//  N*left_cam_pose + right2left_pose + left_cam_params + right_cam_params
			mrpt::aligned_containers<CPose3D>::vector_t left_cam_poses;  // Poses of the origin of coordinates of the pattern wrt the left camera
			CPose3D         right2left_pose;
			CArrayDouble<9> left_cam_params, right_cam_params; // [fx fy cx cy k1 k2 k3 t1 t2]


			// Ctor
			lm_stat_t(
			const TCalibrationStereoImageList & _images,
			const vector<size_t> & _valid_image_pair_indices,
			const vector<TPoint3D> &_obj_points
			) : images(_images), valid_image_pair_indices(_valid_image_pair_indices), obj_points(_obj_points)
			{
				left_cam_poses.assign(images.size(), CPose3D(0,0,1, 0,0,0) );  // Initial
			}

			// Swap:
			void swap(lm_stat_t &o)
			{
				left_cam_poses.swap( o.left_cam_poses );
				std::swap( right2left_pose, o.right2left_pose );
				std::swap( left_cam_params, o.left_cam_params );
				std::swap( right_cam_params, o.right_cam_params );
			}
		};

		/** Data associated to *each observation* in the Lev-Marq. model */
		struct TResidJacobElement
		{
			Eigen::Matrix<double,4,1>  predicted_obs;  //!< [u_l v_l  u_r v_r]: left/right camera pixels
			Eigen::Matrix<double,4,1>  residual;       //!<  = predicted_obs - observations
			Eigen::Matrix<double,4,30> J; //!< Jacobian. 4=the two predicted pixels; 30=Read below for the meaning of these 30 variables
		};

		typedef vector< mrpt::aligned_containers<TResidJacobElement>::vector_t > TResidualJacobianList;

		// Auxiliary functions for the Lev-Marq algorithm:
		double recompute_errors_and_Jacobians(const lm_stat_t & lm_stat, TResidualJacobianList &res_jac, bool use_robust_kernel, double kernel_param);
		void build_linear_system(const TResidualJacobianList  & res_jac,  const vector_size_t & var_indxs, Eigen::VectorXd &minus_g, Eigen::MatrixXd &H);
		void add_lm_increment(const Eigen::VectorXd &eps, const vector_size_t & var_indxs, lm_stat_t &new_lm_stat);

	}
}

#endif // MRPT_VISION_INTERNAL_STEREO_CALIB_H