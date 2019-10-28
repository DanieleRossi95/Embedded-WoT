// asciitable.nut

for(local i = 32; i <= 126; i++) {
	local s = i.tostring();
	if(s.len() < 3)
		s += " ";
	s += " : ";
	s += i.tochar();
	print(s);
}