// showdepth.frag

uniform sampler2D depth;

float color;
float r,g,b;

void main (void)
{
	color = texture2D(depth, gl_TexCoord[0].xy);
	if(color == 1) {
		gl_FragColor = vec4(0,0,0,1);
	}else{
	
		color *= 6;

		if( 0 <= color && color < 1 )		{r = 1;			g = color;		b = 0;}
		else if( 1 <= color && color < 2 )	{r = 2 - color;	g = 1;			b = 0;}
		else if( 2 <= color && color < 3 )	{r = 0;			g = 1;			b = color - 2;}
		else if( 3 <= color && color < 4 )	{r = 0;			g = 4 - color;	b = 1;}
		else if( 4 <= color && color < 5 )	{r = color - 4;	g = 0;			b = 1;}
		else if( 5 <= color && color < 6 )	{r = 1;			g = 0;			b = 6 - color;}
		gl_FragColor = vec4(r,g,b,1);
	}
}
