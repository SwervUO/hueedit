
Usage:
	hueedit --merge huemulsrc huemuladdition huemuldest
		Merges the entries,from huemuladdition, into the huemulsrc, and saved to huemuldest.
			This does not preserve source ids. It merges hues into blank entries
			or appends to the end.

	hueedit --extract huemulsrc huescvfile
		Extracts the entries,from huemulsrt to a csv text file.

	hueedit --empty huemulsrc
		Prints the hue ids that are empty.

	hueedit --compare huemul1 huemul2
		Prints the hueids that are in huemul2 but not present in huemul1.

	hueedit --create huemul huecvsfile
		Creates a huemul from the cvs file.

Note: Color channel values in the csv file are 5 bit (0-31)!!!!!
