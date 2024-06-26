suppressPackageStartupMessages(library("viridis"))

raw_data = read.csv("out", header = TRUE)
quantiles = aggregate(Clocks ~ Id + N + C, raw_data, quantile)
quantiles = quantiles[order(quantiles$N, quantiles$Id, quantiles$C), ]

# Cambiar la escala de los ciclos a millones de ciclos
quantiles$Clocks = quantiles$Clocks / 1e6

get_speedup = function(data, base_id) {
    base = quantiles[quantiles$Id == base_id, ][c('N', 'Clocks')]
    colnames(base) = c('N', 'Clocks_base')

    data = merge(data, base, by = "N")

    # Calcular el speedup
    data$Speedup = data$Clocks_base / data$Clocks

    # Seleccionar las columnas relevantes
    data = data[c('N', 'Id', 'C', 'Clocks', 'Speedup')]
    return (data)
}


create_plot = function(data, x_field, y_field, group_field, intervals = TRUE,
                       legend, legend_pos = "topleft", palette = viridis,
                       save_as = NULL, pdf_height = 7,
                       ...) {
    if (!is.null(save_as)) {
        pdf(save_as, height = pdf_height)
    }

    X = as.matrix(data[x_field])
    Y = as.matrix(data[y_field])
    G = as.matrix(data[group_field])
    Y.q1 = Y[, 2]
    Y.q2 = Y[, 3]
    Y.q3 = Y[, 4]

    colors = palette(length(unique(G)))

    plot(X, Y.q2, pch = 19, type = 'p', col = colors, ...)
    axis(side = 1, at = unique(X))

    for (i in 1:length(unique(G))) {
        g = unique(G)[i];
        
        x = X[G == g]
        y.q1 = Y.q1[G == g]
        y.q2 = Y.q2[G == g]
        y.q3 = Y.q3[G == g]

        points(x, y.q2, type = 'l', col = colors[i], lwd = 2)
        if (intervals) {
            arrows(x0 = x, x1 = x, y0 = y.q1, y1 = y.q3, 
                   code = 3, angle = 90, length = 0.1, col = colors[i])
        }
    }

    legends = sprintf(legend, unique(G))
    legend(legend_pos, legends, fill = colors)

    if (!is.null(save_as)) {
        dev.off()
    }
}  

### Gráfica 1: comparativa general
# Solo mostrar los datos con C = 64 hilos para el programa 4
data1 = quantiles[quantiles$C == 1 | quantiles$C == 64,]

create_plot(data1, 'N', 'Clocks', 'Id', 
            legend = "Experimento: %s", palette = turbo,
            main = "Comparativa general de rendimiento\nentre las diferentes versiones",
            xlab = "Dimensión de las matrices (N)", 
            ylab = "Millones de ciclos de reloj",
            log = 'y',
            save_as = "Graficas/comparativa_general.pdf"
)



### Gráfica 2: Aceleración según el número de hilos (O0) variando N
data20 = quantiles[quantiles$Id == "4o0", ]
data20 = get_speedup(data20, '2  ')

create_plot(data20, 'N', 'Speedup', 'C',
            legend = "C = %d", palette = viridis,
            main = "Aceleración según el número de hilos (4o0)\nrespecto al programa secuencial optimizado (2)",
            xlab = "Dimensión de las matrices (N)", 
            ylab = "Speedup",
            save_as = "Graficas/speedup_hilosO0.pdf"
)




### Gráfica 3: Aceleración según el número de hilos (O2) variando N
data22 = quantiles[quantiles$Id == "4o2", ]
data22 = get_speedup(data22, '2  ')

create_plot(data22, 'N', 'Speedup', 'C',
            legend = "C = %d", palette = mako,
            main = "Aceleración según el número de hilos (4o2)\nrespecto al programa secuencial optimizado (2)",
            xlab = "Dimensión de las matrices (N)", 
            ylab = "Speedup",
            save_as = "Graficas/speedup_hilosO2.pdf"
)



### Gráfica 4: comparativa entre los distintos números de hilos para N máximo
data4 = quantiles[quantiles$N == max(quantiles$N) & grepl('4', quantiles$Id),]
data4 = data4[order(data4$C, data4$Id), ]
data4 = get_speedup(data4, '2  ')


create_plot(data4, 'C', 'Speedup', 'Id', 
            legend = "Experimento: %s", palette = turbo,
            main = "Aceleración según el número de hilos\npara la dimensión máxima de las matrices (N = 3500)\nrespecto al programa secuencial optimizado (2)",
            xlab = "Número de hilos (C)", 
            ylab = "Speedup",
            log = 'x', xaxt = 'n',
            save_as = "Graficas/speedup_hilosNmax.pdf",
            pdf_height = 6
) 



### Gráfica 5: Aceleración de la versión 2 frente a 1o0
data5 = quantiles[grepl('1o0', quantiles$Id) | grepl('^2', quantiles$Id),]
data5 = get_speedup(data5, '1o0')


create_plot(data5, 'N', 'Speedup', 'Id',
            legend = "Experimento: %s", palette = viridis,
            main = "Aceleración de la versión secuencial optimizada (2)\nfrente a la versión inicial compilada con O0 (1o0)",
            xlab = "Dimensión de las matrices (N)", 
            ylab = "Speedup",
            ylim = c(0.5, 4),
            save_as = "Graficas/speedup_2-1o0.pdf",
            pdf_height = 5
)


### Gráfica 6: Aceleración de las versiones 3 y 4 frente a la 2
data6 = quantiles[grepl('^2', quantiles$Id) | grepl('^3', quantiles$Id) | grepl('^4', quantiles$Id),]
data6 = data6[data6$C == 1 | data6$C == 64, ]
data6 = get_speedup(data6, '2  ')

create_plot(data6, 'N', 'Speedup', 'Id',
            legend = "Experimento: %s", palette = plasma,
            main = "Aceleración de las versiones con paralelismo (3 y 4)\nfrente a la versión secuencial optimizada (2)",
            xlab = "Dimensión de las matrices (N)", 
            ylab = "Speedup",
            save_as = "Graficas/speedup_34-2.pdf",
            pdf_height = 6
)



### Gráfica 7: Aceleración manual frente a automática
data7 = quantiles[grepl('1o3', quantiles$Id) | grepl('^2', quantiles$Id) | grepl('^3', quantiles$Id) | grepl('^4', quantiles$Id),]
data7 = data7[data7$C == 1 | data7$C == 64, ]
data7 = get_speedup(data7, '1o3')

create_plot(data7, 'N', 'Speedup', 'Id',
            legend = "Experimento: %s", palette = mako,
            main = "Aceleración de la optimización manual (2, 3 y 4)\nfrente a la optimización automática (1o3)",
            xlab = "Dimensión de las matrices (N)", 
            ylab = "Speedup",
            log = 'y',
            save_as = "Graficas/speedup_234-1o3.pdf",
            pdf_height = 6
)

### Gráfica 8: Eficiencia respecto al número de hilos
data8 = quantiles[grepl('4o0', quantiles$Id) , ]
data8 = get_speedup(data8, '2  ')
data8$Efficiency = data8$Speedup / data8$C
create_plot(data8, 'N', 'Efficiency', 'C',
            legend = "C = %d", palette = viridis,
            legend_pos = "right",
            main = "Eficiencia por hilo del programa paralelo (4o0)\n(Speedup / Número de hilos)",
            xlab = "Dimensión de las matrices (N)", 
            ylab = "Eficiencia",
            save_as = "Graficas/eficiencia.pdf"
)

### Gráfica 9: Aceleración de la versión 2 respecto a la 3
data9 = quantiles[grepl('^2', quantiles$Id) | grepl('^3', quantiles$Id), ]
data9 = get_speedup(data9, '2  ')

create_plot(data9, 'N', 'Speedup', 'Id', intervals = FALSE,
            legend = "Experimento: %s", legend_pos = "topright", palette = viridis,
            main = "Aceleración de la versión con paralelismo a nivel de datos (3)\nfrente a la versión secuencial optimizada (2)",
            xlab = "Dimensión de las matrices (N)", 
            ylab = "Speedup",
            save_as = "Graficas/speedup_3-2.pdf",
            pdf_height = 5
)
