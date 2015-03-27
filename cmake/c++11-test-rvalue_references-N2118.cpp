int foo(int& lvalue)
{
	return 123;
}

int foo(int&& rvalue)
{
	return 321;
}

int main()
{
	int i = 42;
	return ((foo(i) == 123) && (foo(42) == 321)) ? 0 : 1;
}
