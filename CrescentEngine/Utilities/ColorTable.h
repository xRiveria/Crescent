#pragma once
#include <glm/glm.hpp>

//To Tidy Up. 

#define Red = glm::vec3(1.0f, 0.0f, 0.0f);
#define Green = glm::vec3(0.0f, 1.0f, 0.0f);
#define Blue = glm::vec3(0.0f, 0.0f, 1.0f);
#define Yellow = glm::vec3(1.0f, 1.0f, 0.0f);
#define Cyan = glm::vec3(0.0f, 1.0f, 1.0f);
#define Magenta = rgb <1, 0, 1>
#define Clear = rgbf 1
#define White = rgb 1
#define Black = rgb 0

// These grays are useful for fine-tuning lighting color values and for other areas where subtle variations of grays are needed.
// Percentage Grays:

#define Gray05 = White0.05
#define Gray10 = White0.10
#define Gray15 = White0.15
#define Gray20 = White0.20
#define Gray25 = White0.25
#define Gray30 = White0.30
#define Gray35 = White0.35
#define Gray40 = White0.40
#define Gray45 = White0.45
#define Gray50 = White0.50
#define Gray55 = White0.55
#define Gray60 = White0.60
#define Gray65 = White0.65
#define Gray70 = White0.70
#define Gray75 = White0.75
#define Gray80 = White0.80
#define Gray85 = White0.85
#define Gray90 = White0.90
#define Gray95 = White * 0.95

// Others Grays:

#define DimGray = color red 0.329412 green 0.329412 blue 0.329412
#define DimGrey = color red 0.329412 green 0.329412 blue 0.329412
#define Gray = color red 0.752941 green 0.752941 blue 0.752941
#define Grey = color red 0.752941 green 0.752941 blue 0.752941
#define LightGray = color red 0.658824 green 0.658824 blue 0.658824
#define LightGrey = color red 0.658824 green 0.658824 blue 0.658824
#define VLightGray = color red 0.80 green 0.80 blue 0.80
#define VLightGrey = color red 0.80 green 0.80 blue 0.80

#define Aquamarine = color red 0.439216 green 0.858824 blue 0.576471
#define BlueViolet = color red 0.62352 green 0.372549 blue 0.623529
#define Brown = color red 0.647059 green 0.164706 blue 0.164706
#define CadetBlue = color red 0.372549 green 0.623529 blue 0.623529
#define Coral = color red 1.0 green 0.498039 blue 0.0
#define CornflowerBlue = color red 0.258824 green 0.258824 blue 0.435294
#define DarkGreen = color red 0.184314 green 0.309804 blue 0.184314
#define DarkOliveGreen = color red 0.309804 green 0.309804 blue 0.184314
#define DarkOrchid = color red 0.6 green 0.196078 blue 0.8
#define DarkSlateBlue = color red 0.419608 green 0.137255 blue 0.556863
#define DarkSlateGray = color red 0.184314 green 0.309804 blue 0.309804
#define DarkSlateGrey = color red 0.184314 green 0.309804 blue 0.309804
#define DarkTurquoise = color red 0.439216 green 0.576471 blue 0.858824
#define Firebrick = color red 0.556863 green 0.137255 blue 0.137255
#define ForestGreen = color red 0.137255 green 0.556863 blue 0.137255
#define Gold = color red 0.8 green 0.498039 blue 0.196078
#define Goldenrod = color red 0.858824 green 0.858824 blue 0.439216
#define GreenYellow = color red 0.576471 green 0.858824 blue 0.439216
#define IndianRed = color red 0.309804 green 0.184314 blue 0.184314
#define Khaki = color red 0.623529 green 0.623529 blue 0.372549
#define LightBlue = color red 0.74902 green 0.847059 blue 0.847059
#define LightSteelBlue = color red 0.560784 green 0.560784 blue 0.737255
#define LimeGreen = color red 0.196078 green 0.8 blue 0.196078
#define Maroon = color red 0.556863 green 0.137255 blue 0.419608
#define MediumAquamarine = color red 0.196078 green 0.8 blue 0.6
#define MediumBlue = color red 0.196078 green 0.196078 blue 0.8
#define MediumForestGreen = color red 0.419608 green 0.556863 blue 0.137255
#define MediumGoldenrod = color red 0.917647 green 0.917647 blue 0.678431
#define MediumOrchid = color red 0.576471 green 0.439216 blue 0.858824
#define MediumSeaGreen = color red 0.258824 green 0.435294 blue 0.258824
#define MediumSlateBlue = color red 0.498039 blue 1.0
#define MediumSpringGreen = color red 0.498039 green 1.0
#define MediumTurquoise = color red 0.439216 green 0.858824 blue 0.858824
#define MediumVioletRed = color red 0.858824 green 0.439216 blue 0.576471
#define MidnightBlue = color red 0.184314 green 0.184314 blue 0.309804
#define Navy = color red 0.137255 green 0.137255 blue 0.556863
#define NavyBlue = color red 0.137255 green 0.137255 blue 0.556863
#define Orange = color red 1 green 0.5 blue 0.0
#define OrangeRed = color red 1.0 green 0.25
#define Orchid = color red 0.858824 green 0.439216 blue 0.858824
#define PaleGreen = color red 0.560784 green 0.737255 blue 0.560784
#define Pink = color red 0.737255 green 0.560784 blue 0.560784
#define Plum = color red 0.917647 green 0.678431 blue 0.917647
#define Salmon = color red 0.435294 green 0.258824 blue 0.258824
#define SeaGreen = color red 0.137255 green 0.556863 blue 0.419608
#define Sienna = color red 0.556863 green 0.419608 blue 0.137255
#define SkyBlue = color red 0.196078 green 0.6 blue 0.8
#define SlateBlue = color green 0.498039 blue 1.0
#define SpringGreen = color green 1.0 blue 0.498039
#define SteelBlue = color red 0.137255 green 0.419608 blue 0.556863
#define Tan = color red 0.858824 green 0.576471 blue 0.439216
#define Thistle = color red 0.847059 green 0.74902 blue 0.847059
#define Turquoise = color red 0.678431 green 0.917647 blue 0.917647
#define Violet = color red 0.309804 green 0.184314 blue 0.309804
#define VioletRed = color red 0.8 green 0.196078 blue 0.6
#define Wheat = color red 0.847059 green 0.847059 blue 0.74902
#define YellowGreen = color red 0.6 green 0.8 blue 0.196078
#define SummerSky = color red 0.22 green 0.69 blue 0.87
#define RichBlue = color red 0.35 green 0.35 blue 0.67
#define Brass = color red 0.71 green 0.65 blue 0.26
#define Copper = color red 0.72 green 0.45 blue 0.20
#define Bronze = color red 0.55 green 0.47 blue 0.14
#define Bronze2 = color red 0.65 green 0.49 blue 0.24
#define Silver = color red 0.90 green 0.91 blue 0.98
#define BrightGold = color red 0.85 green 0.85 blue 0.10
#define OldGold = color red 0.81 green 0.71 blue 0.23
#define Feldspar = color red 0.82 green 0.57 blue 0.46
#define Quartz = color red 0.85 green 0.85 blue 0.95
#define Mica = color Black // needed in textures.inc
#define NeonPink = color red 1.00 green 0.43 blue 0.78
#define DarkPurple = color red 0.53 green 0.12 blue 0.47
#define NeonBlue = color red 0.30 green 0.30 blue 1.00
#define CoolCopper = color red 0.85 green 0.53 blue 0.10
#define MandarinOrange = color red 0.89 green 0.47 blue 0.20
#define LightWood = color red 0.91 green 0.76 blue 0.65
#define MediumWood = color red 0.65 green 0.50 blue 0.39
#define DarkWood = color red 0.52 green 0.37 blue 0.26
#define SpicyPink = color red 1.00 green 0.11 blue 0.68
#define SemiSweetChoc = color red 0.42 green 0.26 blue 0.15
#define BakersChoc = color red 0.36 green 0.20 blue 0.09
#define Flesh = color red 0.96 green 0.80 blue 0.69
#define NewTan = color red 0.92 green 0.78 blue 0.62
#define NewMidnightBlue = color red 0.00 green 0.00 blue 0.61
#define VeryDarkBrown = color red 0.35 green 0.16 blue 0.14
#define DarkBrown = color red 0.36 green 0.25 blue 0.20
#define DarkTan = color red 0.59 green 0.41 blue 0.31
#define GreenCopper = color red 0.32 green 0.49 blue 0.46
#define DkGreenCopper = color red 0.29 green 0.46 blue 0.43
#define DustyRose = color red 0.52 green 0.39 blue 0.39
#define HuntersGreen = color red 0.13 green 0.37 blue 0.31
#define Scarlet = color red 0.55 green 0.09 blue 0.09

#define Med_Purple = colour red 0.73 green 0.16 blue 0.96
#define Light_Purple = colour red 0.87 green 0.58 blue 0.98
#define Very_Light_Purple = colour red 0.94 green 0.81 blue 0.99