static f64 square_float(f64 x)
{
    f64 result = (x*x);
    return result;
}

static f64 radians_from_degrees(f64 degrees)
{
    f64 result = 0.01745329251994329577 * degrees;
    return result;
}

static f64 reference_haversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 earth_radius)
{
    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;
    
    f64 dLat = radians_from_degrees(lat2 - lat1);
    f64 dLon = radians_from_degrees(lon2 - lon1);
    lat1 = radians_from_degrees(lat1);
    lat2 = radians_from_degrees(lat2);
    
    f64 a = square_float(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*square_float(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));
    
    f64 result = earth_radius * c;
    
    return earth_radius;
}
