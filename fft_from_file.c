#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "C:\\CPP\\fft\\fft.h"
#include "C:\\CPP\\wav_reader\\wav_reader.h"
#include "C:\\CPP\\wav_generator\\wav_generator.h"
#include "C:\\CPP\\signal_analyzer\\signal_analyzer.h"

int get_csv_column(FILE *fp, char *col_header)
{
//	int col = -1;
	int filechar;
	int i = 0;
	int col_header_len = strlen(col_header);
	int col_counter = 0;

//	printf("Length of column header \"%s\": %d\n", col_header, col_header_len);

	while ('\n' != (filechar = getc(fp)))
	{
		if (',' == filechar)
		{
			col_counter++;
//			printf("Increasing column to %d\n", col_counter);
			i = -1; // Reset comparison index
			continue;
		}
		else if (' ' == filechar && -1 == i)
		{
//			printf("Skipping leading space\n");
			continue; // Skip leading spaces
		}
		else if (filechar == col_header[++i])
		{
//			printf("Matched char %c to index %d of col_header\n", filechar, i);
//			i++;
			if (i == (col_header_len - 1))
				return col_counter;
		}
		else
		{
//			printf("Mismatched char %c to col_header[%d]=%c\n", filechar, i, col_header[i]);
			col_counter++;
			i = -1;
			while (',' != (filechar = getc(fp)))
			{
				if ('\n' == filechar)
					return -1;
			}
		}

		/* Does strlen start at 0 or 1? */
//		if (i >= col_header_len || 
//				(-1 != i && filechar != col_header[i]))
//		{
//			while (',' != getc(fp))
//			{
//				if ('\n'
	}

	printf("Reached end of line outside loop\n");
	return -1;
}

double get_next_value(FILE *fp, int col, bool *val_retrieved)
{
	int i;
	int filechar;
	char buf[256] = "";
	*val_retrieved = false;

	/* Assume we're in the middle of the previous line */
	while ('\n' != getc(fp));
	for (i = 0; i < col; i++)
	{
//		printf("At column %d\n", i);
		while (1)
		{
			filechar = getc(fp);
//			printf("%c ", filechar);
			if ('\n' == filechar || EOF == filechar)
			{
//				printf("Reached line-end or EOF\n");
				return 0;
			}
			else if(',' == filechar)
			{
//				printf("Reached a comma\n");
				break;
			}
		}
	}

//	i = 0;
	for (i = 0; i < 255; i++)
	{
		while (' ' == (filechar = getc(fp))); // Kill leading spaces
		if (',' == filechar || '\n' == filechar || EOF == filechar)
		{
//			printf("Terminating buffer\n");
			buf[i] = '\0';
			*val_retrieved = true;
			break;
		}
		else
		{
//			printf("Adding value %c to buf[%d]\n", filechar, i);
			buf[i] = filechar;
		}
	}

//	printf("Returning string %s\n", buf);
//	return atof(buf);
//	printf("Converted to %f\n", retval);
	return atof(buf);
}

typedef struct SimpleLink
{
	double value;
	struct SimpleLink *next;
} SimpleLink;

void add_link(SimpleLink **linked_list, double value)
{
	SimpleLink *new_link = (SimpleLink *)calloc(1, sizeof(*new_link));
	SimpleLink *lp = NULL;

	new_link->value = value;
	if (NULL == *linked_list)
	{
		*linked_list = new_link;
	}
	else
	{
		lp = *linked_list;
		while (NULL != lp->next)
		{
			lp = lp->next;
		}
		lp->next = new_link;
	}
//	printf("New link value: %f\n", lp->next->value);
}

void print_links(SimpleLink *linked_list)
{
	SimpleLink *lp = linked_list;
	while (NULL != lp)
	{
		printf("Link value %f\n", lp->value);
		lp = lp->next;
	}
}

void destroy_links(SimpleLink **linked_list)
{
	SimpleLink *lp = *linked_list;
	while (lp != NULL)
	{
		*linked_list = lp->next;
		free(lp);
		lp = *linked_list;
	}
}

void read_array_from_csv(char *filename, char *col_header, double **array_values, int *num_values)
{
	FILE *fp = fopen(filename, "r");
	int i = 0, j = 0;
	int filechar;
	bool val_retrieved = false;
	double value = 0;
	SimpleLink *linked_list = NULL;
	SimpleLink *lp = NULL;

	*num_values = 0;

//	while ('\n' != (filechar = getc(fp)))
//	{
//		printf("%c ", filechar);
//		i++;
//	}
//	printf("\n");
//	printf("File first line end at index %d\n", i);

	int col = get_csv_column(fp, col_header);

	printf("Found header %s at column %d\n", col_header, col);

//	for (i = 0; i < 10; i++)
//		printf("Value %d: %f\n", i, get_next_value(fp, col));

	i = 0;
	while (1)
	{
		value = get_next_value(fp, col, &val_retrieved);
		i++;	
		if (!val_retrieved)
		{
			printf("Ended at value number %d\n", i);
			break;
		}
		else
		{
			(*num_values)++;
			add_link(&linked_list, value);
//			printf("Got value %f at row %d\n", value, i);
		}
	}

	fclose(fp);

	*array_values = (double *)calloc(*num_values, sizeof(**array_values));
	lp = linked_list;
	for (i = 0; i < *num_values; i++)
	{
//		printf("Assigning value %d\n", lp->value);
		(*array_values)[i] = lp->value;
		lp = lp->next;
	}
	printf("Assigned %d values\n", i);
	
//	print_links(linked_list);
	printf("Last value: %f\n", (*array_values)[i-1]);
	destroy_links(&linked_list);	
}

void print_ft_array_to_csv(char *filename, FourierData *fourier_arrays, long int num_channels)
{
	FILE *fp = fopen(filename, "w+");
	int i,j;

	for (i = 0; i < num_channels; i++)
	{
		fprintf(fp, "Time, Value, Frequency, FT Re, FT Im, FT Mag^2, , ");
	}
	fprintf(fp, "\n");

	for (j = 0; j < fourier_arrays[0].num_samples; j++)
	{
		for (i = 0; i < num_channels; i++)
		{
			fprintf(fp, "%f, %f, %f, %f, %f, %f, , ", fourier_arrays[i].dt * (double) j, 
					fourier_arrays[i].sample_array[j], 
					fourier_arrays[i].sample_frequencies[j], 
					fourier_arrays[i].sample_ft[j].re, fourier_arrays[i].sample_ft[j].im, 
					fourier_arrays[i].sample_power[j]);
		}
		fprintf(fp, "\n");
	}
}

void print_ft_and_ift_arrays_to_csv(char *filename, FourierData *fourier_arrays, long int num_channels)
{
	FILE *fp = fopen(filename, "w+");
	int i,j;

	for (i = 0; i < num_channels; i++)
	{
		fprintf(fp, "Time, Value, Frequency, FT Re, FT Im, FT Mag^2, IFT Re, IFT Im, , ");
	}
	fprintf(fp, "\n");

	for (j = 0; j < fourier_arrays[0].num_samples; j++)
	{
		for (i = 0; i < num_channels; i++)
		{
			fprintf(fp, "%f, %f, %f, %f, %f, %f, %f, %f, , ", fourier_arrays[i].dt * (double) j,
					fourier_arrays[i].sample_array[j],
					fourier_arrays[i].sample_frequencies[j],
					fourier_arrays[i].sample_ft[j].re, fourier_arrays[i].sample_ft[j].im,
					fourier_arrays[i].sample_power[j],
					fourier_arrays[i].sample_ift[j].re, fourier_arrays[i].sample_ift[j].im);
		}
		fprintf(fp, "\n");
	}
}

void fft_from_file(char *filename, bool calculate_ift)
{
	WaveFile wavefile = { 0 };
	FourierData *fourier_transforms;
	PeakList **peak_lists = NULL;

	int i;

	read_wave(&wavefile, filename);

	printf("%ld bytes read\n", wavefile.data_section_size);
	printf("Ch 0, first 10 raw samples: ");
	for (i = 0; i < 10; i++)
		printf("%ld ", wavefile.channel_samples[0][i]);
	printf("\n");

	print_header(wavefile);

	convert_channel_samples_to_sample_doubles(&wavefile, &fourier_transforms);

	printf("Ch 0, first 10 converted samples: ");
	for (i = 0; i < 10; i++)
		printf("%f ", fourier_transforms[0].sample_array[i]);
	printf("\n");

	for (i = 0; i < wavefile.num_channels; i++)
	{
		calculate_ft_arrays(&(fourier_transforms[i]));
	}

	if (calculate_ift)
	{
		for (i = 0; i < wavefile.num_channels; i++)
		{
			calculate_ift_arrays(&(fourier_transforms[i]));
		}

		print_ft_and_ift_arrays_to_csv("FFT_IFFT_output.csv", fourier_transforms, wavefile.num_channels);
	}
	else
	{
		print_ft_array_to_csv("FFT_output.csv", fourier_transforms, wavefile.num_channels);
	}

	peak_lists = (PeakList **)calloc(wavefile.num_channels, sizeof(**peak_lists));
	for (i = 0; i < wavefile.num_channels; i++)
	{
		double threshold = calculate_average(fourier_transforms[i].sample_power,
				fourier_transforms[i].num_samples);
		find_peaks_above_threshold(fourier_transforms[i].sample_power, fourier_transforms[i].num_samples,
				threshold, &(peak_lists[i]));
		printf("Peaks of channel %d\n", i);
		print_peaks(peak_lists[i], fourier_transforms[i].sample_frequencies);
	}

	destroy_wavearrays(&wavefile);
	for (i = 0; i < wavefile.num_channels; i++)
		destroy_fourier_data_arrays(&(fourier_transforms[i]));
}

void notch_filter(FourierData **p_fourier_transforms, double filter_frequency, int num_channels)
{
	int filter_index = filter_frequency * (*p_fourier_transforms)[0].dt
	       	* (*p_fourier_transforms)[0].num_samples;
//	int filter_mirror_index = (*p_fourier_transforms)[0].num_samples - filter_index - 1;
	int filter_mirror_index = (*p_fourier_transforms)[0].num_samples - filter_index;
	int i;

	printf("Filtering frequency %f at index %d, mirror index %d\n", filter_frequency, filter_index,
			filter_mirror_index);

	for (i = 0; i < num_channels; i++)
	{
		(*p_fourier_transforms)[i].sample_ft[filter_index].re = 0;
		(*p_fourier_transforms)[i].sample_ft[filter_index].im = 0;
		(*p_fourier_transforms)[i].sample_ft[filter_mirror_index].re = 0;
		(*p_fourier_transforms)[i].sample_ft[filter_mirror_index].im = 0;
		(*p_fourier_transforms)[i].sample_power[filter_index] = 0;
		(*p_fourier_transforms)[i].sample_power[filter_mirror_index] = 0;
	}
}

void fft_filter_from_file(char *filename, double filter_frequency)
{
	WaveFile wavefile = { 0 };
	FourierData *fourier_transforms = NULL;

	int i;

	read_wave(&wavefile, filename);
	printf("%ld bytes read from file %s\n", wavefile.data_section_size, filename);
	print_header(wavefile);

	printf("Converting wavefile sample shorts to doubles\n");
	convert_channel_samples_to_sample_doubles(&wavefile, &fourier_transforms);

	printf("Calculating DFT of sample data for %d channels\n", wavefile.num_channels);
	for (i = 0; i < wavefile.num_channels; i++)
	{
		calculate_ft_arrays(&(fourier_transforms[i]));
	}

	/* Do the filtering here. Consider keeping original FT data copy for graphing */
	notch_filter(&fourier_transforms, filter_frequency, wavefile.num_channels);

	printf("Calculating IDFT of filtered spectral data\n");
	for (i = 0; i < wavefile.num_channels; i++)
	{
		calculate_ift_arrays(&(fourier_transforms[i]));
	}

	print_ft_and_ift_arrays_to_csv("FFT_filtered_IFFT_output.csv", fourier_transforms, wavefile.num_channels);

	printf("Converting Real component of IDFT to wavefile short int\n");
	convert_sample_doubles_to_channel_shorts(&wavefile, fourier_transforms);
		
	write_wave(wavefile, "FFT_filtered_IFFT_output.wav");

	destroy_wavearrays(&wavefile);
	for (i = 0; i < wavefile.num_channels; i++)
		destroy_fourier_data_arrays(&fourier_transforms[i]);
}

#if 1
int main()
{
//	char filename[128] = "C:\\Users\\PC\\Downloads\\WAV_Samples\\M1F1-int16-Afsp.wav";
//	char filename[128] = "C:\\CPP\\wav_generator\\output_test.wav";
//	char filename[128] = "C:\\CPP\\wav_generator\\440hz_880hz_5s.wav";
//	char filename[128] = "C:\\CPP\\wav_generator\\440hz_880hz_2sec_1chan.wav";
//	char filename[128] = "C:\\CPP\\wav_generator\\A4_C5_2s1c.wav";
/*	char filename[128] = "C:\\CPP\\wav_generator\\C5_Ef5_G5_2s1c.wav";
	fft_from_file(filename, true);*/
//	fft_filter_from_file(filename, 880);
//	fft_filter_from_file(filename, A4);

//	char filename[128] = "C:\\CPP\\fft_from_file\\FFT_IFFT_output.csv";
	char filename[128] = "C:\\Users\\PC\\Downloads\\sax_fft.csv";
	char col_header[128] = "FT Mag^2";
	double *array_values = NULL;
	int num_values = 0;
	double average = 0;
	PeakList *peak_list = NULL;

	read_array_from_csv(filename, col_header, &array_values, &num_values);

	average = calculate_average(array_values, num_values);
	printf("Using average threshold %f\n", average);

	find_peaks_above_threshold(array_values, num_values, average, &peak_list);
	print_peaks(peak_list, NULL); // Get a frequency array too

	free(array_values);
	return 0;
}
#endif
