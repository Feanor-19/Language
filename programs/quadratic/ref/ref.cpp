#include <stdio.h>
#include <math.h>

float get_dicsriminant( float a, float b, float c )
{
    float discriminant = b*b - 4 * a*c;

    return discriminant;
}

// ax + b = 0, a != 0
float get_linear_root( float a, float b )
{
    float res = -b / a;
    return res;
}

// ax + b = 0
void solve_linear( float a, float b )
{
    if ( a == 0 )
    {
        if ( b == 0)
        {
            printf( "Well, there is kinda the whole infinity and a small carriage of roots...\n" );
        }
        else
        {
            printf( "Quite interesting, that's also not a linear equation. It's not an "
                    "equation at all: there are simply no roots, even imaginary ones!\n" );
        }
    }
    else
    {
        float root = get_linear_root( a, b );

        printf( "Here is the only linear root:\n" );
        printf( "%f\n", root );
    }
}

//! ax^2 + bx + c = 0, a != 0
float get_quadratic_first_root( float a, float b, float discriminant)
{
    float res = (-b + sqrt( discriminant ) ) / (2 * a);
}

float get_quadratic_number_of_roots( float discriminant )
{
    float res = 0;
    if ( discriminant > 0 )
    {
        res = 2;
        return res;
    }

    if ( discriminant == 0 )
    {
        res = 1;
        return res;
    }

    if ( discriminant < 0 )
    {
        res = 0;
        return res;
    }
}

// ax^2 + bx + c = 0
void solve_quadratic( float a, float b, float c )
{
    if ( a == 0)
    {
        printf( "Well, that's not quite a quadratic equation, "
                "smells more like a linear one. Anyway...\n" );

        solve_linear( b, c );
    }
    else
    {
        float discriminant = get_dicsriminant( a, b, c );
        printf( "Here is the discriminant of your equation:\n" );
        printf( "%f\n", discriminant );

        float n_of_roots = get_quadratic_number_of_roots( discriminant );
        printf( "The number of roots is:\n" );
        printf( "%f\n", n_of_roots );

        if ( n_of_roots == 0)
        {
            printf( "No real roots!\n" );
        }
        else
        {
            float first_root = get_quadratic_first_root( a, b, discriminant );

            printf( "Aaaand here is the first root:\n" );
            printf( "%f\n", first_root );

            if ( n_of_roots == 2 )
            {
                float second_root = -b / a - first_root;
                printf( "And the second root is:\n" );
                printf( "%f\n", second_root );
            }
        }
    }
}

int main()
{
    printf( "Hi, I can solve quadratic equations, like ax^2 + bx + c = 0! "
            "Do you want to make a try? [1/0]\n" );

    float ans = 0;
    scanf( "%f", &ans );

    while ( ans != 0 )
    {
        printf( "Please, enter coefficient a:\n" );
        float a = 0;
        scanf( "%f", &a );

        printf( "Please, enter coefficient b:\n" );
        float b = 0;
        scanf( "%f", &b );

        printf( "Please, enter coefficient c:\n" );
        float c = 0;
        scanf( "%f", &c );

        solve_quadratic( a, b, c );

        printf( "Done! Do you want to solve another equation? [1/0]\n" );
        scanf( "%f", &ans );
    }

    printf( "Goodbye!\n" );
}
