SELECT city.name as "City", city.population as "City Population",country.population as "Country Population" FROM City
JOIN Country ON Country.Code = City.CountryCode
order by cast(city.population as real)/cast(country.population as real) desc
limit 20