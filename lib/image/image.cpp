#include <boost/gil/extension/dynamic_image/dynamic_image_all.hpp>

#include "geometry/Rectangle_2_point_iterator.h"
#include "image/image_inc.hpp"

#ifdef GEOMETRY_RECTANGLE_2_H
double rjmcmc::image::error(const Rectangle_2 &r) const {
	return 0;
	/*
	geometry::Rectangle_2_point_iterator<Rectangle_2> it(r);
	double res = 0;
	for (; !it.end() ; ++it)
	{
		double ndvi = ((boost::gil::const_view(m_image))(it.x()-x0,it.y()-y0));
		res += (ndvi-150);
	}
	return res;
	*/
}
#endif // GEOMETRY_RECTANGLE_2_H

#ifdef GEOMETRY_CIRCLE_2_H
double rjmcmc::image::error(const Circle_2  &c) const
{
	double r = geometry::radius(c);
	double x = c.center().x()-x0;
	double y = c.center().y()-y0;
	int i0 = std::max(0,(int) (x-r));
	int i1 = std::min((int) (m_image.width()-1),(int) (x+r+1));
	int j0 = std::max(0,(int) (y-r));
	int j1 = std::min((int) (m_image.height()-1),(int) (y+r+1));
	double res = 0;
	for ( int i = i0 ; i<=i1 ; ++i)
	{
		for ( int j = j0 ; j<=j1 ; ++j)
		{
			if ((x-i)*(x-i)+(y-j)*(y-j)>r*r) continue;
			double ndvi = ((boost::gil::const_view(m_image))(i,j));
			res +=(140-ndvi) ;
		}
	}
		
	return res;
}
#endif // GEOMETRY_CIRCLE_2_H

