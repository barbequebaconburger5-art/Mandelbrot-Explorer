#version 400 core
out vec4 FragColor;

uniform vec2 u_resolution;
uniform int u_max_iterazioni;

// Riceviamo i componenti High e Low dei double dal C++
uniform float u_centroX_hi;
uniform float u_centroX_lo;
uniform float u_centroY_hi;
uniform float u_centroY_lo;
uniform float u_ampiezza_hi;
uniform float u_ampiezza_lo;

void main() {
    // Riassembliamo i pezzi ottenendo la massima precisione a 64 bit hardware
    double centroX = double(u_centroX_hi) + double(u_centroX_lo);
    double centroY = double(u_centroY_hi) + double(u_centroY_lo);
    double ampiezza = double(u_ampiezza_hi) + double(u_ampiezza_lo);

    // Coordinate del pixel convertite subito in double precision
    dvec2 st = dvec2(gl_FragCoord.xy) / dvec2(u_resolution);
    
    // Proporzioni dello schermo
    double aspetto = double(u_resolution.x) / double(u_resolution.y);
    
    // Calcoliamo i confini matematici del frattale in double precision
    double minX = centroX - (ampiezza * aspetto) / 2.0;
    double maxX = centroX + (ampiezza * aspetto) / 2.0;
    double minY = centroY - ampiezza / 2.0;
    double maxY = centroY + ampiezza / 2.0;
    
    // Mappatura precisa del pixel corrente
    double reale = minX + (maxX - minX) * st.x;
    double immag = minY + (maxY - minY) * st.y;
    
    dvec2 c = dvec2(reale, immag);
    dvec2 z = dvec2(0.0, 0.0);
    
    int iterazione = 0;
    // Il ciclo di calcolo viaggia a 64-bit FP64 dentro la GPU
    while (dot(z, z) <= 4.0 && iterazione < u_max_iterazioni) {
        double x = z.x * z.x - z.y * z.y + c.x;
        double y = 2.0 * z.x * z.y + c.y;
        z = dvec2(x, y);
        iterazione++;
    }
    
    // Algoritmo di colorazione fluida
    if (iterazione == u_max_iterazioni) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        // CORREZIONE: Convertiamo in float il modulo quadro prima di calcolare il logaritmo
        float dot_z = float(dot(z, z));
        float log_zn = log(dot_z) / 2.0;
        float nu = log(log_zn / log(2.0)) / log(2.0);
        float iterazione_fluida = float(iterazione) + 1.0 - nu;
        float t = iterazione_fluida / 40.0;
        
        float r = 0.5 * (1.0 + sin(t + 0.0));
        float g = 0.5 * (1.0 + sin(t + 1.04));
        float b = 0.5 * (1.0 + sin(t + 2.09));
        
        FragColor = vec4(r, g, b, 1.0);
    }
}