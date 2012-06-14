int main()
{
	static_assert(1 < 0, "this should fail");
	return 0;
}
