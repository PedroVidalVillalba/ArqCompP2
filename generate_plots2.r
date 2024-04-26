suppressPackageStartupMessages(library("viridis"))

raw_data = read.csv("out2", header = TRUE)
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


create_plot = function(data, x_field, y_field, line_field, 
                       legend, legend_pos = "topleft", palette = viridis,
                       save_as = NULL,
                       ...) {
    if (!is.null(save_as)) {
        pdf(save_as)
    }

    X = as.matrix(data[x_field])
    Y = as.matrix(data[y_field])
    L = as.matrix(data[line_field])
    Y.q1 = Y[, 2]
    Y.q2 = Y[, 3]
    Y.q3 = Y[, 4]

    colors = palette(length(unique(L)))

    plot(X, Y.q2, pch = 19, type = 'p', col = colors, ...)
    axis(side = 1, at = unique(X))

    for (i in 1:length(unique(L))) {
        l = unique(L)[i];
        
        x = X[L == l]
        y.q1 = Y.q1[L == l]
        y.q2 = Y.q2[L == l]
        y.q3 = Y.q3[L == l]

        points(x, y.q2, type = 'l', col = colors[i], lwd = 2)
        arrows(x0 = x, x1 = x, y0 = y.q1, y1 = y.q3, 
               code = 3, angle = 90, length = 0.1, col = colors[i])
    }

    legends = sprintf(legend, unique(L))
    legend(legend_pos, legends, fill = colors)

    if (!is.null(save_as)) {
        dev.off()
    }
}  

quantiles[quantiles$N == 3500,]
quantiles = get_speedup(quantiles, '1o0')

create_plot(quantiles, 'N', 'Speedup', 'Id',
            legend = "Experimento: %s")
