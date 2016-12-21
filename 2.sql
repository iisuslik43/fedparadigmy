SELECT country.name, rate FROM Country
JOIN literacyrate ON Country.Code = literacyrate.CountryCode
where rate = (
	select max(rate) from literacyrate
	where countrycode = code
)
group by country.name
order by rate desc
