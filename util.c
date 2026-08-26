/* 16 MHz SYSCLK */
void delayMs(int n){
	int i;
	for (; n > 0; n--)
	for (i = 0; i < 1067; i++)
	{
		__asm("NOP");
	}
}