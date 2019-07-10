#include <stdio.h>
#include <stdbool.h>
#include "C:\\CPP\\fft\\fft.h"
#include "C:\\CPP\\wav_reader\\wav_reader.h"
#include "C:\\CPP\\wav_generator\\wav_generator.h"

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

int main()
{
//	char filename[128] = "C:\\Users\\PC\\Downloads\\WAV_Samples\\M1F1-int16-Afsp.wav";
//	char filename[128] = "C:\\CPP\\wav_generator\\output_test.wav";
//	char filename[128] = "C:\\CPP\\wav_generator\\440hz_880hz_5s.wav";
//	char filename[128] = "C:\\CPP\\wav_generator\\440hz_880hz_2sec_1chan.wav";
//	char filename[128] = "C:\\CPP\\wav_generator\\A4_C5_2s1c.wav";
	char filename[128] = "C:\\CPP\\wav_generator\\C5_Ef5_G5_2s1c.wav";
	fft_from_file(filename, true);
//	fft_filter_from_file(filename, 880);
//	fft_filter_from_file(filename, A4);

	return 0;
}
