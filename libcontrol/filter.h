#ifndef FILTER_H
#define FILTER_H

#include "common_headers.h"

// Retour le niveau de bruit dans une image pour determiner si on peut utiliser cette frame 
int get_image_noise_level(const cv::Mat&);


#endif // FILTER_H