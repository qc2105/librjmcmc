#ifndef __SHP_VISITOR_HPP__
#define __SHP_VISITOR_HPP__

#include <string>
#include <shapefil.h>

#ifdef  GEOMETRY_RECTANGLE_2_H
template<typename K>
SHPObject* shp_create_object(const geometry::Rectangle_2<K>& r)
{
    double x[5], y[5];
    for(unsigned int i=0; i<4; ++i)
    {
        typename geometry::Rectangle_2<K>::Point_2 p = r.point(i);
        x[i] = geometry::to_double(p.x());
        y[i] = geometry::to_double(p.y());
    }
    x[4]=x[0];
    y[4]=y[0];

    return SHPCreateSimpleObject(SHPT_POLYGON, 5, x, y, NULL);
}
#endif //  GEOMETRY_RECTANGLE_2_H

#ifdef  GEOMETRY_CIRCLE_2_H
template<typename K>
SHPObject* shp_create_object(const geometry::Circle_2<K>& c)
{
    enum { N=20 };
    double x[N+1], y[N+1];
    typename geometry::Circle_2<K>::Point_2 p = c.center();
    double x0 = geometry::to_double(p.x());
    double y0 = geometry::to_double(p.y());
    double r  = geometry::to_double(c.radius());
    for(unsigned int i=0; i<N; ++i)
    {
        double theta = (2*M_PI*i)/N;
        x[i] = x0+r*cos(theta);
        y[i] = y0+r*sin(theta);
    }
    x[N]=x[0];
    y[N]=y[0];

    return SHPCreateSimpleObject(SHPT_POLYGON, N+1, x, y, NULL);
}
#endif

struct shp_writer
{

public:
    shp_writer(const std::string& file) { m_file = SHPCreate(file.c_str(), SHPT_POLYGON); }
    ~shp_writer() { SHPClose(m_file); }

    typedef void result_type;

    inline operator bool() { return m_file; }

    template<typename T> void operator()(const T& t) const {
        SHPObject* object = shp_create_object(t);
        SHPComputeExtents(object);
        SHPWriteObject( m_file, -1, object);
    }

private:
    SHPHandle m_file;
};

#include <iostream>
#include <iomanip>
#include <sstream>
#include "rjmcmc/variant.hpp"

namespace simulated_annealing {
    namespace shp {

        class shp_visitor
        {
        public:
            shp_visitor(const std::string& prefix) : m_iter(0), m_prefix(prefix) {}

            void init(int, int s)
            {
                m_save = s;
                m_iter = 0;
            }

            template<typename Configuration, typename Sampler>
            void begin(const Configuration& config, const Sampler&, double)
            {
                m_iter = 0;
                save(config);
            }

            template<typename Configuration, typename Sampler>
            void end(const Configuration& config, const Sampler&, double)
            {
                save(config);
            }

            template<typename Configuration, typename Sampler>
            void visit(const Configuration& config, const Sampler&, double) {
                if((++m_iter)%m_save==0)
                    save(config);
            }
        private:
            unsigned int m_save, m_iter;
            std::string m_prefix;

            template<typename Configuration>
            void save(const Configuration& config) const
            {
                std::ostringstream oss;
                oss << m_prefix << std::setw(15) << std::setfill('0') << m_iter << ".shp";
                shp_writer writer(oss.str());
                if(!writer)
                {
                    std::cout << "\tUnable to create SHP " << oss.str() << std::endl;
                    return;
                }
                typename Configuration::const_iterator it = config.begin(), end = config.end();
                for (; it != end; ++it)
                {
                    rjmcmc::apply_visitor(writer,config[it]);
                }
            }
        };

    } // namespace shp
} // namespace simulated_annealing

#endif // __WX_CHART_VISITOR_HPP__
