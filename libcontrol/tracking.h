// mvlar.h : Include file for standard system include files,
// or project specific include files.
#include "common_headers.h"
#pragma once
#include <iostream>



// SimpleObjectTracker permet de faire le suivit simple d'un object d'une forme géométrique simple et d'une couleur simple
class SimpleObjectTracker
{
	struct images_info
	{
		cv::Mat image;
		int		noise;
	};

	bool						get_interval_between = false; // si vrai quand on fait le range on prend les valeur entre le treshold sinon on prend les valeurs a l'exterieur

	cv::Scalar					low_thresh {0,134,91};
	cv::Scalar					high_thresh {12,255,255};

	cv::Size					object_size { 20 , 20};

	int							interval_execute = 100;

	cv::Mat						best_image;
	int							current_noise = 1000;
private:
	

public:
	// ajoute une nouvelle image et retourne true si le gap d'execution est passer
	bool addImage(cv::Mat& miam);
	// retourne l'image la moins bruité avec le threshold d'appliquer
	cv::Mat getBestThreshOutput();

	bool isModeIntervalBetween() const {
		return get_interval_between;
	}

	cv::Scalar getLowTresh() { return low_thresh; }
	void setLowTresh(cv::Scalar v) { low_thresh = v;}
	cv::Scalar getHighTresh() { return high_thresh; }
	void setHighTresh(cv::Scalar v) { high_thresh = v;}
	cv::Size getObjectSize() { return object_size; }
	void setObjectSize(cv::Size s) { object_size= s;}
	int getIntervalMS() { return interval_execute; }
	void setIntervalMS(int v) { interval_execute = v;}


};




void traitementImage(const cv::Mat&,Mat &retour);




// TODO: Reference additional headers your program requires here.
