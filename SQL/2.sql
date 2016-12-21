SELECT country.name, rate FROM Country
JOIN literacyrate ON Country.Code = literacyrate.CountryCode
where rate = (
	select rate from literacyrate
	where countrycode = code
	order by year desc 
	limit 1
)
group by country.name
order by rate desc
limit 1
