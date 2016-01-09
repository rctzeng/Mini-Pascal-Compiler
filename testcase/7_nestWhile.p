program test(in, out, err);
var a, b, c : integer;
begin
    c := 0;
    a := 2;
    while(a <= 9) do
    begin
        b := 1;
        while(b <= 9) do
        begin
            c := c+(a*b);
            b := b+1
        end;
        a := a+1
    end;

    printInt(c)
end.

