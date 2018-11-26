// mvlar.h : Include file for standard system include files,
// or project specific include files.
#pragma once
#include <iostream>

#include "common_headers.h"
#include "chrono.h"




// SimpleObjectTracker permet de faire le suivit simple d'un object d'une forme géométrique simple et d'une couleur simple
class SimpleObjectTracker
{
	struct images_info
	{
		cv::Mat image;
		int		noise;
	};

	struct obj_info
	{
		float		pixel_area;
		cv::Point	position;
		TimePoint	at_time;
	};

	bool						get_interval_between = false; // si vrai quand on fait le range on prend les valeur entre le treshold sinon on prend les valeurs a l'exterieur

	cv::Scalar					low_thresh_1 {0,134,91};
	cv::Scalar					high_thresh_1 {12,255,255};

	cv::Scalar					low_thresh_2 {168,134,91};
	cv::Scalar					high_thresh_2 {179,255,255};


	cv::Size					object_size { 20 , 20};

	Chrono						chrono;
	int							interval_execute = 100;

	cv::Mat						best_image;
	int							current_noise = 1000;

	int							noise_parse_max = 1000;
	int							noise_contour_max = 50;

	std::vector<obj_info>		object_info_list; // contient les dernieres informations de positions sur l'object

public:
	void reset()
	{
		chrono.setDuration(milliseconds(interval_execute));
		chrono.reset();
	}
	// ajoute une nouvelle image et retourne true si le gap d'execution est passer
	bool addImage(cv::Mat& miam);
	// retourne l'image la moins bruité avec le threshold d'appliquer
	cv::Mat getBestThreshOutput(bool& is_ready);


	void setModeThreeshold(bool interval_between)
	{
		get_interval_between = interval_between;
	}
	bool isModeIntervalBetween() const {
		return get_interval_between;
	}

	cv::Scalar getLowTresh() const { return low_thresh_1; }
	void setLowTresh(cv::Scalar v) { low_thresh_1 = v;}
	cv::Scalar getHighTresh() const { return high_thresh_1; }
	void setHighTresh(cv::Scalar v) { high_thresh_1 = v;}

	cv::Scalar getLowTresh2() const { return low_thresh_2; }
	void setLowTresh2(cv::Scalar v) { low_thresh_2 = v; }
	cv::Scalar getHighTresh2() const { return high_thresh_2; }
	void setHighTresh2(cv::Scalar v) { high_thresh_2 = v; }

	cv::Size getObjectSize() const { return object_size; }
	void setObjectSize(cv::Size s) { object_size= s;}

	int getIntervalMS() const { return interval_execute; }
	void setIntervalMS(int v) { interval_execute = v;}

	void traitementThresholdExterieur(const cv::Mat& frame, cv::Mat& retour);
	void traitementThreshold(const cv::Mat& frame, cv::Mat& retour);

	bool tryFoundObject(const cv::Mat& img);

	std::vector<obj_info>& getObjectsInfo() {
		return object_info_list;
	}

	obj_info getLastObjectInfo()
	{
		if (object_info_list.empty())
			return {};
		return object_info_list.at(object_info_list.size() - 1);
	}

};




void traitementImage(const cv::Mat&,Mat &retour);




// TODO: Reference additional headers your program requires here.
