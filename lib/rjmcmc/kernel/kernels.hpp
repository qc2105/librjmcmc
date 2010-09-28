#ifndef KERNELS_HPP_
#define KERNELS_HPP_

#include "rjmcmc/random.hpp"
#include "rjmcmc/random.hpp"
#include "rjmcmc/kernel/transform.hpp"

template< typename IsValid >
class generator {
    typedef boost::variate_generator<rjmcmc::generator&, boost::uniform_real<> > die_type;
    mutable die_type m_die;
    IsValid m_is_valid;
public:
    generator(const IsValid& is_valid) :
            m_die(rjmcmc::random(), boost::uniform_real<>(0,1)),
            m_is_valid(is_valid) {}

    typedef double result_type;

#ifdef GEOMETRY_RECTANGLE_2_H
    template<typename K> result_type operator()(geometry::Rectangle_2<K> &r) const
    {
        typedef typename K::Point_2 Point_2;
        typedef typename K::Vector_2 Vector_2;
        const Iso_rectangle_2& bbox = m_is_valid.bbox();
        float x0 = bbox.min().x();
        float y0 = bbox.min().y();
        float dx = bbox.max().x()-x0;
        float dy = bbox.max().y()-y0;
        float in [5] = { m_die(), m_die(), m_die(), m_die(), m_die() };
        float out[5];
	float m[5] = { dx, dy, 0.1*dx, 0.1*dy, 1 };
        diagonal_transform<5,float> trans(m);
	trans.apply(in,out);
        // do {
        Point_2 p(x0+out[0], y0+out[1]);
        Vector_2 v(out[2], out[3]);
        r = geometry::Rectangle_2<K>(p, v, out[4]);
        // } while (!m_is_valid(r));
        return trans.abs_jacobian(in);
    }
#endif

#ifdef GEOMETRY_CIRCLE_2_H
    template<typename K> result_type operator()(geometry::Circle_2<K> &c) const
    {
        typedef typename K::Point_2 Point_2;
        const Iso_rectangle_2& bbox = m_is_valid.bbox();
        float x0 = bbox.min().x();
        float y0 = bbox.min().y();
        float dx = bbox.max().x()-x0;
        float dy = bbox.max().y()-y0;
        double radius = 0.5*geometry::min(dx,dy);
        // do {
        Point_2 p(x0+dx*m_die(), y0+dy*m_die());
        c = geometry::Circle_2<K>(p, m_die()*radius);
        // } while (!m_is_valid(c));
        return 1./(dx*dy*radius);
    }
#endif

    struct pdf_visitor {
        IsValid m_is_valid;
        typedef double result_type;
        pdf_visitor(const IsValid& is_valid) : m_is_valid(is_valid) {}
#ifdef GEOMETRY_RECTANGLE_2_H
        template<typename K> result_type operator()(const geometry::Rectangle_2<K> &r) const
        {
            return 1.;
        }
#endif
#ifdef GEOMETRY_CIRCLE_2_H
        template<typename K> result_type operator()(const geometry::Circle_2<K> &r) const
        {
            const Iso_rectangle_2& bbox = m_is_valid.bbox();
            float dx = bbox.max().x()-bbox.min().x();
            float dy = bbox.max().y()-bbox.min().y();
            double radius = 0.5*geometry::min(dx,dy);
            return 1./(dx*dy*radius);
	}
#endif
    };
    inline pdf_visitor pdf() const { return pdf_visitor(m_is_valid); }
};

template< typename IsValid >
class modifier {
    typedef boost::variate_generator<rjmcmc::generator&, boost::uniform_real<> > float_die_type;
    typedef boost::variate_generator<rjmcmc::generator&, boost::uniform_smallint<> > int_die_type;
    mutable float_die_type m_dief;
    mutable int_die_type   m_die4;
    double m_p_translation;
    double m_dx, m_dy;
    IsValid m_is_valid;
public:
    modifier(const IsValid& is_valid) :
            m_dief(rjmcmc::random(), boost::uniform_real<>(0,1)),
            m_die4(rjmcmc::random(), boost::uniform_smallint<>(0,3)),
            m_p_translation(0.5),
            m_dx(20), m_dy(20), m_is_valid(is_valid) {}

    typedef double result_type;

    template<typename T0, typename T1>
    double operator()(const T0 &t0, T1 &t1) const {
        return 0;
    }

#ifdef GEOMETRY_RECTANGLE_2_H
    template<typename K> result_type operator()(
    const geometry::Rectangle_2<K> &r,
    geometry::Rectangle_2<K> &res
    ) const
    {
        typedef typename K::Vector_2 Vector_2;
        if(m_dief()<m_p_translation) {
            res = r.scaled_edge(m_die4(),exp(0.5-m_dief()));
        } else {
            Vector_2 v(m_dx*(m_dief()-0.5),m_dy*(m_dief()-0.5));
            res = r.rotation_scaled_corner(m_die4(), v);
        }
        if(!m_is_valid(res)) return 0.;
        return 1.; // TODO
    }
    /*
  template<typename K> result_type operator()(
                                               const geometry::Rectangle_2<K> &r,
                                               std::pair<geometry::Rectangle_2<K>,geometry::Rectangle_2<K> > &res
                                             ) const
  {
    int i = m_die4();
    float f = m_dief();
    float g = m_dief()*(1-f);
    res.first  = r.scaled_edge(i  ,f);
    res.second = r.scaled_edge(i+2,g);
    if (!(m_is_valid(res.first) && m_is_valid(res.second) )) return 0;
    return 1.; // TODO
  }
  
  template<typename K> result_type operator()(
                                               const std::pair<geometry::Rectangle_2<K>,geometry::Rectangle_2<K> > &r,
                                               geometry::Rectangle_2<K> &res
                                             ) const
  {
    res = r.first.rotate(m_die4()).merge(r.second.rotate(m_die4()));
    if (!(m_is_valid(res))) return 0;
    return 1.; // TODO
  }
#endif // GEOMETRY_RECTANGLE_2_H
  
#ifdef GEOMETRY_CIRCLE_2_H
  template<typename K> result_type operator()(
                                               const geometry::Circle_2<K> &c,
                                               geometry::Circle_2<K> &res
                                             ) const
  {
    typedef typename K::Vector_2 Vector_2;
    if(m_dief()<m_p_translation) {
      Vector_2 v(m_dx*(m_dief()-0.5),m_dy*(m_dief()-0.5));
      double d2 = v.squared_length();
      double d = sqrt(d2);
      if (m_die4()%2 && c.squared_radius()>d2) d = -d;
      res = geometry::Circle_2<K>(c.center()+v,geometry::radius(c)+d);
    } else {
      res = geometry::Circle_2<K>(c.center(),geometry::radius(c)*exp(0.5-m_dief()));
    }
    if(!m_is_valid(res)) return 0.;
    return 1.; // TODO
  }
#endif // GEOMETRY_CIRCLE_2_H
  
#if defined(GEOMETRY_CIRCLE_2_H) && defined(GEOMETRY_RECTANGLE_2_H)
  
  template<typename K> result_type operator()(
                                               const geometry::Rectangle_2<K> &r,
                                               geometry::Circle_2<K> &c
                                             ) const
  {
    double radius = sqrt(r.normal().squared_length());
    double ratio  = std::abs(r.ratio());
    if(ratio<1) radius *= ratio;
    c = geometry::Circle_2<K>(r.center(),radius);
    if(!m_is_valid(c)) return 0.;
    return 1.; // TODO
  }
  
  template<typename K> result_type operator()(
                                               const geometry::Circle_2<K> &c,
                                               geometry::Rectangle_2<K> &r
                                             ) const
  {
    typedef typename K::Vector_2 Vector_2; 
    Vector_2 v((2*m_dief()-1),(2*m_dief()-1));
    v = v*(geometry::radius(c)/sqrt(v.squared_length()));
    r = geometry::Rectangle_2<K>(c.center(),v,1);
    if (!m_is_valid(r)) return 0;
    return 1.; // TODO
  }
  
  template<typename K> result_type operator()(
                                               const geometry::Rectangle_2<K> &r,
                                               std::pair<geometry::Rectangle_2<K>,geometry::Circle_2<K> > &res
                                             ) const
  {
    typedef typename K::Vector_2 Vector_2;
		// todo: verifier que ca fait pas n'importe quoi...
    int i = m_die4();
    float f = m_dief();
    float g = m_dief();
    float h = m_dief();
    Vector_2 n = r.normal(i)*std::min(1.,std::abs(r.ratio()));
    if(i%2) n = n*(1./r.ratio());
    Vector_2 v(-n.x()+n.y(),-n.y()-n.x());
    res.first  = r.scaled_edge(i,f).scaled_edge(i+1,g);
    res.second = geometry::Circle_2<K>(r[i+3]+v*h,h*std::sqrt(n.squared_length()));
    if (!(m_is_valid(res.first) && m_is_valid(res.second) )) return 0;
    return 1.; // TODO
  }*/
#endif // defined(GEOMETRY_CIRCLE_2_H) && defined(GEOMETRY_RECTANGLE_2_H)

};

#endif /*KERNELS_HPP_*/