#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"
#include "bitmap.h"
#include <vector>
#include <cstdint>
#include <chrono>
#include <iomanip>
#include <thread>
#include <iostream>

class camera {
public:
    double aspect_ratio = 1.0;  // Ratio of image width over height
    int    image_width = 800;  // Rendered image width in pixel count
    int    samples_per_pixel = 100;   // Count of random samples for each pixel
	int    max_depth = 10; // Maximum recursion depth for ray tracing
   
    double vfov = 90;
    point3 lookfrom = point3(0, 0, 0);
	point3 lookat = point3(0, 0, -1);
	vec3   vup = vec3(0, 1, 0);

    double defocus_angle = 0;
    double focus_dist = 10;

	std::vector<uint8_t> pixelBuffer; // Buffer for pixel data

	std::vector<uint8_t> renderSection(const hittable& world, const int start, const int end) {
		// Render a section of the image from start to end
        std::vector<uint8_t> sectionPixels;
		for (int j = start; j < end; j++) {
			for (int i = 0; i < image_width; i++) {
				color pixel_color(0, 0, 0);
				for (int sample = 0; sample < samples_per_pixel; sample++) {
					ray r = get_ray(i, j);
					pixel_color += ray_color(r, max_depth, world);
				}
				write_color(std::cout, pixel_samples_scale * pixel_color, sectionPixels);
			}
		}
		return sectionPixels;
	}

    void render(const hittable& world) {
        initialize();

        auto start = std::chrono::steady_clock::now();

        const int num_threads = std::thread::hardware_concurrency();
        const int rows_per_thread = image_height / num_threads;
        std::clog << "\rRendering...";

        std::vector<std::vector<uint8_t>> sections(num_threads);
        std::vector<std::thread> threads;

        for (int t = 0; t < num_threads; ++t) {
            int row_start = t * rows_per_thread;
            int row_end = (t == num_threads - 1) ? image_height : (t + 1) * rows_per_thread;

            threads.emplace_back([&, t, row_start, row_end]() {
                sections[t] = renderSection(world, row_start, row_end);
                });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        // Merge all sections into pixelBuffer
        for (const auto& section : sections) {
            pixelBuffer.insert(pixelBuffer.end(), section.begin(), section.end());
        }

        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "\rElapsed time: " << elapsed_seconds.count() << "s\n";
        std::clog << "\rDone.\n";

        Bitmap::saveBMP("image.bmp", pixelBuffer, image_width, image_height);
    }

private:
    int    image_height;   // Rendered image height
	double pixel_samples_scale; // Scale factor for sum of pixel samples
    point3 center;         // Camera center
    point3 pixel00_loc;    // Location of pixel 0, 0
    vec3   pixel_delta_u;  // Offset to pixel to the right
    vec3   pixel_delta_v;  // Offset to pixel below
	vec3   u, v, w;        // Camera frame basis vectors
    vec3   defocus_disk_u;   // Defocus disk H radius
	vec3   defocus_disk_v;   // Defocus disk V radius

    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

		pixel_samples_scale = 1.0 / samples_per_pixel;

        center = lookfrom;

        // Determine viewport dimensions.
        auto theta = degrees_to_radians(vfov);
		auto h = std::tan(theta / 2);
		auto viewport_height = 2.0 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width) / image_height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors.
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    ray get_ray(int i, int j) const {
        // Construct a camera ray originating from the defocus disk and directed at a randomly
        // sampled point around the pixel location i, j.

        auto offset = sample_square();
        auto pixel_sample = pixel00_loc
            + ((i + offset.x()) * pixel_delta_u)
            + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    vec3 sample_square() const {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }


    color ray_color(const ray& r, int depth, const hittable& world) const {
        // If we've exceeded the ray bounce limit, no more light is gathered.
        if (depth <= 0)
            return color(0, 0, 0);

        hit_record rec;

        if (world.hit(r, interval(0.001, infinity), rec)) {
            ray scattered;
            color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, depth - 1, world);
            return color(0, 0, 0);
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    }
};

#endif