#include <stdio.h>
#include <mpi.h>
#include <time.h>

#define N 140000
double tiempo_inicio;
double tiempo_termino;

void Imprimir(int *array,int length) //print array elements
{
    int i=0;
    for(i=0;i<length;i++)
        printf("%d\n",array[i]);
}

int * merge(int *v1, int n1, int *v2, int n2)
{
	int i,j,k;
	int * result;

	result = (int *)malloc((n1+n2)*sizeof(int));

	i=0; j=0; k=0;
	while(i<n1 && j<n2)
		if(v1[i]<v2[j])
		{
			result[k] = v1[i];
			i++; k++;
		}
		else
		{
			result[k] = v2[j];
			j++; k++;
		}
	if(i==n1)
		while(j<n2)
		{
			result[k] = v2[j];
			j++; k++;
		}
	else
		while(i<n1)
		{
			result[k] = v1[i];
			i++; k++;
		}
	return result;
}

void shellsort (int numbers[], int array_size)
{
    	int i, j, increment, temp;
	increment = 3;
	while (increment > 0)
	{
		for (i=0; i < array_size; i++)
		{
			j = i;
			temp = numbers[i];
			while ((j >= increment) && (numbers[j-increment] > temp))
			{
				numbers[j] = numbers[j - increment];
				j = j - increment;
			}
			numbers[j] = temp;
		}
		if (increment/2 != 0)
			increment = increment/2;
		else if (increment == 1)
			increment = 0;
		else
			increment = 1;
	}
}

int main(int argc, char **argv)
{
	int * data;
	int * chunk;
	int * other;
	int m,n=N;
	int id,p;
	int s;
	int i;
	int step;
	MPI_Status status;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&id);
	MPI_Comm_size(MPI_COMM_WORLD,&p);

	if(id==0)
	{
		int r;
		s = n/p;
		r = n%p;
		data = (int *)malloc((n+p-r)*sizeof(int));
		printf("llenando vector (%d datos)...\n",N);
		for(i=0;i<n;i++)
			data[i] = N - i;
		if(r!=0)
		{
			for(i=n;i<n+p-r;i++)
  			   data[i]=0;
			s=s+1;
		}
		printf("Ejecutando shellsort paralelo...\n");
		tiempo_inicio = clock();

		MPI_Bcast(&s,1,MPI_INT,0,MPI_COMM_WORLD);
		chunk = (int *)malloc(s*sizeof(int));
		MPI_Scatter(data,s,MPI_INT,chunk,s,MPI_INT,0,MPI_COMM_WORLD);
		shellsort(chunk,s);
	}


/**************************** worker task ************************************/
	else
	{
		MPI_Bcast(&s,1,MPI_INT,0,MPI_COMM_WORLD);
		chunk = (int *)malloc(s*sizeof(int));
		MPI_Scatter(data,s,MPI_INT,chunk,s,MPI_INT,0,MPI_COMM_WORLD);
		shellsort(chunk,s);
	}

	step = 1;
	while(step<p)
	{
		if(id%(2*step)==0)
		{
			if(id+step<p)
			{
				MPI_Recv(&m,1,MPI_INT,id+step,0,MPI_COMM_WORLD,&status);
				other = (int *)malloc(m*sizeof(int));
				MPI_Recv(other,m,MPI_INT,id+step,0,MPI_COMM_WORLD,&status);
				chunk = merge(chunk,s,other,m);
				s = s+m;
			}
		}
		else
		{
			int near = id-step;
			MPI_Send(&s,1,MPI_INT,near,0,MPI_COMM_WORLD);
			MPI_Send(chunk,s,MPI_INT,near,0,MPI_COMM_WORLD);
			break;
		}
		step = step*2;
	}
	if(id==0)
	{
		tiempo_termino = clock();
		printf("Tiempo de ejecucion = %f segundos\n",(tiempo_termino-tiempo_inicio)/CLOCKS_PER_SEC);
		//Imprimir(chunk,N);
	}
	MPI_Finalize();
}
