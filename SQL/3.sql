SELECT City.Name FROM City
Join Capital on Capital.cityid = city.id
Join country on Capital.CountryCode = Country.code
where country.name = "Malaysia"