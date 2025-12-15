USE GD1C2025
GO

CREATE SCHEMA LOS_DBS
GO

CREATE SYNONYM Maestra FOR gd_esquema.Maestra;
GO

--FUNCIONES
CREATE FUNCTION 
LOS_DBS.fn_GetCalle (@Direccion NVARCHAR(255))
RETURNS NVARCHAR(255)
AS
BEGIN
    RETURN LEFT(@Direccion, CHARINDEX(' N° ', @Direccion) - 1);
END;
GO

CREATE FUNCTION
LOS_DBS.fn_GetNumero (@Direccion NVARCHAR(255))
RETURNS DECIMAL(18,0)
AS
BEGIN
    RETURN CAST(SUBSTRING(@Direccion, CHARINDEX(' N° ', @Direccion) + 4, LEN(@Direccion)) AS DECIMAL(18,0));
END;
GO

--TABLAS
CREATE TABLE LOS_DBS.Direccion(
	Direccion_Codigo DECIMAL(18,0) NOT NULL  IDENTITY(1,1),
	Direccion_Calle nvarchar(255) NOT NULL,
	Direccion_Numero DECIMAL(18,0) NOT NULL
);
GO

CREATE TABLE LOS_DBS.Localidad(
	Localidad_Codigo DECIMAL(18,0) NOT NULL  IDENTITY(1,1),
	Localidad_Nombre nvarchar(255) NOT NULL
);
GO

CREATE TABLE LOS_DBS.Provincia(
	Provincia_Codigo DECIMAL(18,0) NOT NULL IDENTITY(1,1),
	Provincia_Nombre nvarchar(255) NOT NULL
);
GO

CREATE TABLE LOS_DBS.Cliente(
    Cliente_Codigo DECIMAL(18,0) NOT NULL IDENTITY(1,1),
    Cliente_Provincia DECIMAL(18,0) NOT NULL,
    Cliente_Localidad DECIMAL(18,0) NOT NULL,
    Cliente_Dni BIGINT NOT NULL,
    Cliente_Nombre NVARCHAR(255) NOT NULL,
    Cliente_Apellido NVARCHAR(255) NOT NULL,
    Cliente_Direccion DECIMAL(18,0) NOT NULL,
    Cliente_FechaNacimiento DATETIME2(6) NOT NULL,
    Cliente_Mail NVARCHAR(255) NOT NULL,
    Cliente_Telefono NVARCHAR(255) NOT NULL
);
GO

CREATE TABLE LOS_DBS.EstadoPedido(
	Estado_Codigo DECIMAL(18,0) NOT NULL IDENTITY(1,1),
	Estado_Descripcion NVARCHAR(255) NOT NULL
);
GO

CREATE TABLE LOS_DBS.Sucursal (
	sucu_nro BIGINT NOT NULL,
	id_provincia DECIMAL(18,0) NOT NULL,
	id_localidad DECIMAL(18,0) NOT NULL,
	sucu_direccion DECIMAL(18,0) NOT NULL,
	sucu_telefono NVARCHAR(255) NOT NULL,
	sucu_mail NVARCHAR(255) NOT NULL
);
GO

CREATE TABLE LOS_DBS.Pedido(
  Pedido_Numero DECIMAL(18,0) NOT NULL,
  Pedido_Estado DECIMAL(18,0) NOT NULL,
  Pedido_Cliente DECIMAL(18,0) NOT NULL,
  Pedido_Sucursal BIGINT NOT NULL,
  Pedido_Fecha DATETIME2(6) NOT NULL,
  Pedido_Total DECIMAL(32,2) NOT NULL
);
GO

CREATE TABLE LOS_DBS.PedidoCancelacion (
    PedidoCan_Codigo DECIMAL(18,0) NOT NULL IDENTITY(1,1),
	PedidoCan_Fecha DATETIME2(6) NOT NULL,
	PedidoCan_Motivo VARCHAR(255) NULL,
	PedidoCan_Cliente DECIMAL(18,0) NOT NULL,
	PedidoCan_Pedido DECIMAL(18,0) NOT NULL
);
GO

CREATE TABLE LOS_DBS.Material(
	mat_codigo DECIMAL(18,0) NOT NULL IDENTITY(1,1),
	mat_nombre NVARCHAR(255) NOT NULL,
	mat_descripcion NVARCHAR(255) NOT NULL,
	mat_precio DECIMAL(38,2) NOT NULL
);
GO

CREATE TABLE LOS_DBS.Relleno(
	mat_codigo DECIMAL(18,0) NOT NULL,
	relle_densidad DECIMAL(38,2) NULL
);
GO

CREATE TABLE LOS_DBS.Madera (
    mat_codigo DECIMAL(18,0) NOT NULL,
    mad_color NVARCHAR(255) NOT NULL,
    mad_dureza NVARCHAR(255) NOT NULL
);
GO

CREATE TABLE LOS_DBS.Tela (
    mat_codigo DECIMAL(18,0) NOT NULL,
    tel_color NVARCHAR(255) NOT NULL,
    tel_textura NVARCHAR(255) NOT NULL
);
GO

CREATE TABLE LOS_DBS.Medida_Sillon (
    si_medida DECIMAL(18,0) NOT NULL IDENTITY(1,1),
    si_medida_alto DECIMAL(18,2) NOT NULL,
    si_medida_ancho DECIMAL(18,2) NOT NULL,
    si_medida_profundidad DECIMAL(18,2) NOT NULL,
    si_medida_precio DECIMAL(18,2) NOT NULL
);
GO

CREATE TABLE LOS_DBS.Modelo_Sillon (
    si_modelo_codigo DECIMAL(18,0) NOT NULL,
	si_modelo NVARCHAR(255) NOT NULL,
    si_modelo_descripcion NVARCHAR(255) NOT NULL,
    si_modelo_precio DECIMAL(18,2) NOT NULL
);
GO

CREATE TABLE LOS_DBS.Sillon (
    si_codigo DECIMAL(18,0) NOT NULL,
    si_medida DECIMAL(18,0) NOT NULL,
    si_modelo DECIMAL(18,0) NOT NULL
);
GO

CREATE TABLE LOS_DBS.MaterialPorSillon (
    mat_codigo DECIMAL(18,0) NOT NULL,
    si_codigo DECIMAL(18,0) NOT NULL
);
GO

CREATE TABLE LOS_DBS.Proveedor (
	prov_cuit NVARCHAR(255) NOT NULL,
	id_provincia DECIMAL(18,0) NOT NULL,
	id_localidad DECIMAL(18,0) NOT NULL,
	prov_direccion DECIMAL(18,0) NOT NULL,
	prov_razon_social NVARCHAR(255) NOT NULL,
	prov_telefono NVARCHAR(255) NOT NULL,
	prov_mail NVARCHAR(255) NOT NULL
);
GO

CREATE TABLE LOS_DBS.Compra (
    comp_numero DECIMAL(18,0) NOT NULL,
    comp_fecha DATETIME2(6) NOT NULL,
    comp_total DECIMAL(18,2) NOT NULL,
    comp_sucursal BIGINT NOT NULL,
    comp_proveedor NVARCHAR(255) NOT NULL
);
GO

CREATE TABLE LOS_DBS.Detalle_compra (
    detalle_compra DECIMAL(18,0) NOT NULL IDENTITY(1,1),
    det_comp_cantidad DECIMAL(18,0) NOT NULL,
    det_comp_precio DECIMAL(18,2) NOT NULL,
    det_comp_subtotal DECIMAL(18,2) NOT NULL,
    comp_numero DECIMAL(18,0) NOT NULL,
    mat_codigo DECIMAL(18,0) NOT NULL
);
GO

CREATE TABLE LOS_DBS.DetallePedido(
	Detalle_Codigo DECIMAL(18,0) NOT NULL IDENTITY(1,1),
	Detalle_Cantidad BIGINT NOT NULL,
	Detalle_Precio DECIMAL(18,2) NOT NULL,
	Detalle_Sillon DECIMAL (18,0) NOT NULL,
	Detalle_Pedido DECIMAL(18,0) NOT NULL,
	Detalle_SubTotal DECIMAL(18,2) NOT NULL
);
GO

CREATE TABLE LOS_DBS.Factura(
	Factura_Numero DECIMAL(18,0) NOT NULL,
	Factura_Fecha DATETIME2(6) NULL,
	Factura_Cliente DECIMAL(18,0) NOT NULL,
	Factura_Sucursal BIGINT NOT NULL
);
GO

CREATE TABLE LOS_DBS.DetalleFactura(
	Detalle_Codigo DECIMAL(18,0) NOT NULL IDENTITY(1,1),
	Detalle_Cantidad BIGINT NOT NULL,
	Detalle_Precio DECIMAL(18,2) NOT NULL,
	Detalle_Subtotal DECIMAL(18,2) NOT NULL,
	Detalle_Factura DECIMAL (18,0) NOT NULL,
	Detalle_Pedido DECIMAL(18,0) NOT NULL
);
GO

CREATE TABLE LOS_DBS.Envio(
	Envio_Numero DECIMAL(18,0)NOT NULL,
	Envio_Fecha_Programada DATETIME2(6) NOT NULL,
	Envio_Fecha DATETIME2(6) NULL,
	Envio_Importe_Traslado DECIMAL(18,2) NULL,
	Envio_Importe_Subida DECIMAL(18,2) NULL,
	Envio_Total DECIMAL(18,2) NULL,
	Envio_Factura DECIMAL(18,0) NOT NULL
);
GO

--CONSTRAINS

ALTER TABLE LOS_DBS.Localidad ADD CONSTRAINT PK_Localidad PRIMARY KEY (Localidad_Codigo);
GO

ALTER TABLE LOS_DBS.Direccion ADD CONSTRAINT PK_Direccion PRIMARY KEY (Direccion_Codigo);
GO

ALTER TABLE LOS_DBS.Provincia ADD CONSTRAINT PK_Provincia PRIMARY KEY (Provincia_Codigo);
GO

ALTER TABLE LOS_DBS.EstadoPedido ADD CONSTRAINT PK_EstadoPedido PRIMARY KEY (Estado_Codigo);
GO

ALTER TABLE LOS_DBS.Cliente ADD CONSTRAINT PK_Cliente PRIMARY KEY (Cliente_Codigo);
ALTER TABLE LOS_DBS.Cliente
ADD CONSTRAINT FK_Cliente_Direccion FOREIGN KEY (Cliente_Direccion)
   REFERENCES LOS_DBS.Direccion(Direccion_Codigo);
ALTER TABLE LOS_DBS.Cliente
ADD CONSTRAINT FK_Cliente_Localidad FOREIGN KEY (Cliente_Localidad)
	REFERENCES LOS_DBS.Localidad(Localidad_Codigo);
ALTER TABLE LOS_DBS.Cliente 
ADD CONSTRAINT FK_Cliente_Provincia FOREIGN KEY (Cliente_Provincia)
	REFERENCES LOS_DBS.Provincia(Provincia_Codigo);
GO

ALTER TABLE LOS_DBS.Sucursal ADD CONSTRAINT PK_sucursal PRIMARY KEY(sucu_nro);
ALTER TABLE LOS_DBS.Sucursal ADD CONSTRAINT FK_sucursal_provincia FOREIGN KEY(id_provincia)
	REFERENCES LOS_DBS.Provincia (Provincia_Codigo);
ALTER TABLE LOS_DBS.Sucursal ADD CONSTRAINT FK_sucursal_localidad FOREIGN KEY(id_localidad)
	REFERENCES LOS_DBS.Localidad(Localidad_Codigo);
ALTER TABLE LOS_DBS.Sucursal ADD CONSTRAINT FK_sucursal_direccion FOREIGN KEY(sucu_direccion)
	REFERENCES LOS_DBS.Direccion (Direccion_Codigo);
GO

ALTER TABLE LOS_DBS.Pedido ADD CONSTRAINT PK_Pedido PRIMARY KEY (Pedido_Numero)
ALTER TABLE LOS_DBS.Pedido
ADD CONSTRAINT FK_Pedido_Estado FOREIGN KEY (Pedido_Estado)
	REFERENCES LOS_DBS.EstadoPedido (Estado_Codigo)
ALTER TABLE LOS_DBS.Pedido
ADD CONSTRAINT FK_Pedido_Sucursal FOREIGN KEY (Pedido_Sucursal)
	REFERENCES LOS_DBS.Sucursal (sucu_nro)
ALTER TABLE LOS_DBS.Pedido
ADD CONSTRAINT FK_Pedido_Cliente FOREIGN KEY (Pedido_Cliente)
	REFERENCES LOS_DBS.Cliente (Cliente_Codigo)
GO

ALTER TABLE LOS_DBS.Proveedor ADD CONSTRAINT PK_proveedor PRIMARY KEY(prov_cuit);
ALTER TABLE LOS_DBS.Proveedor ADD CONSTRAINT FK_proveedor_provincia FOREIGN KEY(id_provincia)
	REFERENCES LOS_DBS.Provincia (Provincia_Codigo);
ALTER TABLE LOS_DBS.Proveedor ADD CONSTRAINT FK_proveedor_localidad FOREIGN KEY(id_localidad)
	REFERENCES LOS_DBS.Localidad(Localidad_Codigo);
ALTER TABLE LOS_DBS.Proveedor ADD CONSTRAINT FK_proveedor_direccion FOREIGN KEY(prov_direccion)
	REFERENCES LOS_DBS.Direccion (Direccion_Codigo);
GO

ALTER TABLE LOS_DBS.PedidoCancelacion ADD CONSTRAINT PK_PedidoCancelacion PRIMARY KEY (PedidoCan_Codigo)
ALTER TABLE LOS_DBS.PedidoCancelacion ADD CONSTRAINT FK_PedidoCancelacion_Cliente FOREIGN KEY(PedidoCan_Cliente)
	REFERENCES LOS_DBS.Cliente (Cliente_Codigo)
ALTER TABLE LOS_DBS.PedidoCancelacion ADD CONSTRAINT FK_PedidoCancelacion_Pedido FOREIGN KEY (PedidoCan_Pedido)
	REFERENCES LOS_DBS.Pedido (Pedido_Numero)
GO

ALTER TABLE LOS_DBS.Material ADD CONSTRAINT PK_material PRIMARY KEY(mat_codigo);
GO

ALTER TABLE LOS_DBS.Relleno ADD CONSTRAINT PK_relleno PRIMARY KEY(mat_codigo);
ALTER TABLE LOS_DBS.Relleno ADD CONSTRAINT FK_relleno_material FOREIGN KEY (mat_codigo)
REFERENCES LOS_DBS.Material(mat_codigo)
GO

ALTER TABLE LOS_DBS.Madera ADD CONSTRAINT PK_madera PRIMARY KEY (mat_codigo);
ALTER TABLE LOS_DBS.Madera ADD CONSTRAINT FK_madera_material FOREIGN KEY (mat_codigo)
        REFERENCES LOS_DBS.Material(mat_codigo)
GO

ALTER TABLE LOS_DBS.Tela ADD CONSTRAINT PK_tela PRIMARY KEY (mat_codigo);
ALTER TABLE LOS_DBS.Tela ADD CONSTRAINT FK_tela_material FOREIGN KEY (mat_codigo)
        REFERENCES LOS_DBS.Material(mat_codigo)
GO

ALTER TABLE LOS_DBS.Medida_Sillon ADD CONSTRAINT PK_medida_sillon PRIMARY KEY (si_medida);
GO

ALTER TABLE LOS_DBS.Modelo_Sillon ADD CONSTRAINT PK_modelo_sillon PRIMARY KEY (si_modelo_codigo);
GO

ALTER TABLE LOS_DBS.Sillon ADD CONSTRAINT PK_sillon PRIMARY KEY (si_codigo);
ALTER TABLE LOS_DBS.Sillon ADD CONSTRAINT FK_sillon_medida FOREIGN KEY (si_medida)
    REFERENCES LOS_DBS.Medida_Sillon (si_medida);
ALTER TABLE LOS_DBS.Sillon ADD CONSTRAINT FK_sillon_modelo FOREIGN KEY (si_modelo)
    REFERENCES LOS_DBS.Modelo_Sillon (si_modelo_codigo);
GO

ALTER TABLE LOS_DBS.MaterialPorSillon ADD CONSTRAINT PK_material_por_sillon PRIMARY KEY (mat_codigo, si_codigo);
ALTER TABLE LOS_DBS.MaterialPorSillon ADD CONSTRAINT FK_mps_material FOREIGN KEY (mat_codigo)
    REFERENCES LOS_DBS.material (mat_codigo);
ALTER TABLE LOS_DBS.MaterialPorSillon ADD CONSTRAINT FK_mps_sillon FOREIGN KEY (si_codigo)
    REFERENCES LOS_DBS.Sillon (si_codigo);
GO

ALTER TABLE LOS_DBS.Compra ADD CONSTRAINT PK_compra PRIMARY KEY (comp_numero);
ALTER TABLE LOS_DBS.Compra ADD CONSTRAINT FK_compra_proveedor FOREIGN KEY(comp_proveedor)
	REFERENCES LOS_DBS.Proveedor (prov_cuit);
ALTER TABLE LOS_DBS.Compra ADD CONSTRAINT FK_compra_sucursal FOREIGN KEY(comp_sucursal)
	REFERENCES LOS_DBS.Sucursal (sucu_nro);
GO

ALTER TABLE LOS_DBS.Detalle_compra ADD CONSTRAINT PK_detalle_compra PRIMARY KEY (detalle_compra);
ALTER TABLE LOS_DBS.Detalle_compra ADD CONSTRAINT FK_detalle_compra_compra FOREIGN KEY (comp_numero)
    REFERENCES LOS_DBS.Compra (comp_numero);
ALTER TABLE LOS_DBS.Detalle_compra ADD CONSTRAINT FK_detalle_compra_material FOREIGN KEY (mat_codigo)
    REFERENCES LOS_DBS.Material (mat_codigo);
GO

ALTER TABLE LOS_DBS.DetallePedido ADD CONSTRAINT PK_DetallePedido PRIMARY KEY (Detalle_Codigo)
ALTER TABLE LOS_DBS.DetallePedido ADD CONSTRAINT FK_DetallePedido_Pedido FOREIGN KEY(Detalle_Pedido)
	REFERENCES LOS_DBS.Pedido (Pedido_Numero)
ALTER TABLE LOS_DBS.DetallePedido ADD CONSTRAINT FK_DetallePedido_Sillon FOREIGN KEY (Detalle_Sillon)
	REFERENCES LOS_DBS.Sillon (si_codigo)
GO 

ALTER TABLE LOS_DBS.Factura ADD CONSTRAINT PK_Factura PRIMARY KEY (Factura_Numero)
ALTER TABLE LOS_DBS.Factura ADD CONSTRAINT FK_Factura_Cliente FOREIGN KEY(Factura_Cliente)
	REFERENCES LOS_DBS.Cliente (Cliente_Codigo)
ALTER TABLE LOS_DBS.Factura ADD CONSTRAINT FK_Factura_Sucursal FOREIGN KEY(Factura_Sucursal)
	REFERENCES LOS_DBS.Sucursal (sucu_nro)
GO

ALTER TABLE LOS_DBS.DetalleFactura ADD CONSTRAINT PK_DetalleFactura PRIMARY KEY(Detalle_Codigo)
ALTER TABLE LOS_DBS.DetalleFactura ADD CONSTRAINT FK_DetalleFactura_DetallePedido FOREIGN KEY(Detalle_Pedido)
	REFERENCES LOS_DBS.DetallePedido (Detalle_Codigo)
ALTER TABLE LOS_DBS.DetalleFactura ADD CONSTRAINT FK_DetalleFactura_Factura FOREIGN KEY(Detalle_Factura)
	REFERENCES LOS_DBS.Factura (Factura_Numero)
GO

ALTER TABLE LOS_DBS.Envio ADD CONSTRAINT PK_Envio PRIMARY KEY (Envio_Numero)
ALTER TABLE LOS_DBS.Envio ADD CONSTRAINT FK_Envio_Factura FOREIGN KEY (Envio_Factura)
	REFERENCES LOS_DBS.Factura (Factura_Numero)
GO

----------------------------------------------------------PROCEDURES----------------------------------------------------------------------------------------

create procedure LOS_DBS.migrar_Provincia
as 
begin 
	INSERT INTO LOS_DBS.Provincia (Provincia_Nombre) 
	select 
		Provincia_Nombre
		from (
		SELECT Cliente_Provincia as Provincia_Nombre 
		FROM gd_esquema.Maestra
		where Cliente_Provincia is not null 
		UNION
		SELECT Proveedor_Provincia
		FROM gd_esquema.Maestra
		where Proveedor_Provincia is not null 
		UNION
		SELECT Sucursal_Provincia
		FROM gd_esquema.Maestra
		where Sucursal_Provincia is not null 
	)Provincia
end;
GO


create procedure LOS_DBS.migrar_Localidad
as
begin 

	INSERT INTO LOS_DBS.Localidad (Localidad_Nombre) 
	select 
		Localidad_Nombre
	from (
		SELECT Cliente_Localidad as Localidad_Nombre 
		FROM gd_esquema.Maestra
		where Cliente_Localidad is not null 
		UNION
		SELECT Proveedor_Localidad
		FROM gd_esquema.Maestra
		where Proveedor_Localidad is not null 
		UNION
		SELECT Sucursal_Localidad
		FROM gd_esquema.Maestra
		where Sucursal_Localidad is not null
)Localidad
end;
GO


create procedure LOS_DBS.migrar_Direccion
as 
begin 
INSERT INTO LOS_DBS.Direccion (Direccion_Calle, Direccion_Numero) 
select 
		Direccion_Calle,
		Direccion_Numero
from (
		SELECT
		LOS_DBS.fn_GetCalle(Cliente_Direccion) AS Direccion_Calle,
		LOS_DBS.fn_GetNumero(Cliente_Direccion) AS Direccion_Numero
		FROM gd_esquema.Maestra
		where Cliente_Direccion is not null
		UNION
		SELECT
		LOS_DBS.fn_GetCalle(Proveedor_Direccion) AS Direccion_Calle,
		LOS_DBS.fn_GetNumero(Proveedor_Direccion) AS Direccion_Numero
		FROM gd_esquema.Maestra
		where Proveedor_Direccion is not null
		UNION
		SELECT
		LOS_DBS.fn_GetCalle(Sucursal_Direccion) AS Direccion_Calle,
		LOS_DBS.fn_GetNumero(Sucursal_Direccion) AS Direccion_Numero
		FROM gd_esquema.Maestra
		where Sucursal_Direccion is not null
)as Direccion
end;
GO


create procedure LOS_DBS.migrar_EstadoPedido
as 
begin 
INSERT INTO LOS_DBS.EstadoPedido (Estado_descripcion) 
select distinct
		Pedido_Estado 
	from gd_esquema.Maestra
		where Pedido_Estado is not null
end;
GO


create procedure LOS_DBS.migrar_Cliente
as 
begin
    INSERT INTO LOS_DBS.Cliente (
        Cliente_Provincia,
        Cliente_Localidad,
        Cliente_Dni,
        Cliente_Nombre,
        Cliente_Apellido,
        Cliente_Direccion,
        Cliente_FechaNacimiento,
        Cliente_Mail,
        Cliente_Telefono
    )
	SELECT DISTINCT
        prov.Provincia_Codigo AS Cliente_Provincia,
        loc.Localidad_Codigo AS Cliente_Localidad,
        ma.Cliente_Dni,
        ma.Cliente_Nombre,
        ma.Cliente_Apellido,
        dir.Direccion_Codigo AS Cliente_Direccion,
        ma.Cliente_FechaNacimiento,
        ma.Cliente_Mail,
        ma.Cliente_Telefono
	from gd_esquema.Maestra ma
	inner join LOS_DBS.Provincia prov on prov.Provincia_Nombre = ma.Cliente_Provincia
	inner join LOS_DBS.Direccion dir on dir.Direccion_Calle = LOS_DBS.fn_GetCalle(ma.Cliente_Direccion) and
											dir.Direccion_Numero = LOS_DBS.fn_GetNumero(ma.Cliente_Direccion)
	inner join LOS_DBS.Localidad loc on loc.Localidad_Nombre = ma.Cliente_Localidad
where Cliente_Dni is not null
end;
GO


CREATE PROCEDURE LOS_DBS.migrar_Proveedor
AS
BEGIN
    INSERT INTO LOS_DBS.Proveedor (prov_cuit,id_provincia,id_localidad,prov_direccion,prov_razon_social,prov_telefono,prov_mail)
    SELECT DISTINCT
        Proveedor_Cuit,
        p.Provincia_Codigo,
        l.Localidad_Codigo,
        d.Direccion_Codigo,
        Proveedor_RazonSocial,
        Proveedor_Telefono,
        Proveedor_Mail
    FROM Maestra
    JOIN LOS_DBS.Provincia p ON p.Provincia_Nombre = Proveedor_Provincia
    JOIN LOS_DBS.Localidad l ON l.Localidad_Nombre = Proveedor_Localidad
    JOIN LOS_DBS.Direccion d ON 
        d.Direccion_Calle = LOS_DBS.fn_GetCalle(Proveedor_Direccion)AND 
        d.Direccion_Numero = LOS_DBS.fn_GetNumero(Proveedor_Direccion)
    WHERE Proveedor_Cuit IS NOT NULL
END
GO


CREATE PROCEDURE LOS_DBS.migrar_Sucursal
AS
BEGIN
    INSERT INTO LOS_DBS.Sucursal (sucu_nro,id_provincia,id_localidad,sucu_direccion,sucu_telefono,sucu_mail)
    SELECT DISTINCT
        Sucursal_NroSucursal,
        p.Provincia_Codigo,
        l.Localidad_Codigo,
        d.Direccion_Codigo,
        Sucursal_telefono,
        Sucursal_mail
    FROM Maestra 
    JOIN LOS_DBS.Provincia p ON 
		p.Provincia_Nombre = Sucursal_Provincia
    JOIN LOS_DBS.Localidad l ON 
		l.Localidad_Nombre = Sucursal_Localidad
	JOIN LOS_DBS.Direccion d ON 
		d.Direccion_Calle = LOS_DBS.fn_GetCalle(Sucursal_Direccion) AND
		d.Direccion_Numero = LOS_DBS.fn_GetNumero(Sucursal_Direccion)
    WHERE Sucursal_NroSucursal IS NOT NULL
END
GO


create procedure LOS_DBS.migrar_Pedido
as 
begin
    INSERT INTO LOS_DBS.Pedido (
         Pedido_Numero,
		Pedido_Estado,
		Pedido_Cliente,
		Pedido_Sucursal,
		Pedido_Fecha,
		Pedido_Total
    )
	select 
		Pedido_Numero ,
		Pedido_Estado,
		Pedido_Cliente,
		Pedido_Sucursal,
		Pedido_Fecha,
		Pedido_Total
from (
	select distinct
		ma.Pedido_Numero as Pedido_Numero,
		es.Estado_Codigo as Pedido_Estado,
		cli.cliente_Codigo as Pedido_Cliente,
		ma.Sucursal_NroSucursal as Pedido_Sucursal,
		ma.Pedido_Fecha as Pedido_Fecha,
		ma.Pedido_Total as Pedido_Total
	from gd_esquema.Maestra ma
	inner join LOS_DBS.Cliente cli on cli.Cliente_Dni = ma.Cliente_Dni
	inner join LOS_DBS.EstadoPedido es on es.Estado_Descripcion = ma.Pedido_Estado
where ma.Pedido_Numero is not null) Pedido
end;
GO


create procedure LOS_DBS.migrar_PedidoCancelacion
as 
begin
    INSERT INTO LOS_DBS.PedidoCancelacion (
		PedidoCan_Fecha,
		PedidoCan_Motivo,
		PedidoCan_Cliente,
		PedidoCan_Pedido
    )
	select
		PedidoCan_Fecha ,
		PedidoCan_Motivo,
		PedidoCan_Cliente,
		PedidoCan_Pedido
from (
	select
		ma.Pedido_Cancelacion_Fecha as PedidoCan_Fecha,
		ma.Pedido_Cancelacion_Motivo as PedidoCan_Motivo,
		cli.Cliente_Codigo as PedidoCan_Cliente,
		ma.Pedido_Numero as PedidoCan_Pedido

	from gd_esquema.Maestra ma
	join LOS_DBS.Cliente cli on cli.Cliente_Dni = ma.Cliente_Dni
where ma.Pedido_Cancelacion_Fecha is not null and 
      ma.Pedido_Numero is not null) PedidoCan
end;
GO


CREATE PROC LOS_DBS.migrar_Material AS
BEGIN
	INSERT INTO LOS_DBS.Material (mat_nombre,mat_descripcion,mat_precio)
	SELECT DISTINCT
		m.Material_Nombre,
		m.Material_Descripcion,
		m.Material_Precio
	FROM Maestra m
	WHERE m.Material_Nombre IS NOT NULL AND 
	m.Material_Descripcion IS NOT NULL AND 
	m.Material_Precio IS NOT NULL
END
GO


CREATE PROCEDURE LOS_DBS.migrar_Relleno
AS
BEGIN

    INSERT INTO LOS_DBS.Relleno (mat_codigo, relle_densidad)
    SELECT DISTINCT
        m.mat_codigo,
        Relleno_Densidad
    FROM Maestra
    JOIN LOS_DBS.Material m ON 
	m.mat_nombre = Material_Nombre AND 
	m.mat_descripcion = Material_Descripcion AND 
	m.mat_precio = Material_Precio
    WHERE Material_Tipo = 'Relleno' 
END
GO


CREATE PROCEDURE LOS_DBS.migrar_Maderas
AS
BEGIN

    INSERT INTO LOS_DBS.Madera (mat_codigo, mad_color, mad_dureza)
    SELECT DISTINCT
        m.mat_codigo,
        Madera_Color,
        Madera_Dureza
    FROM Maestra
    JOIN LOS_DBS.Material m ON 
		m.mat_nombre = Material_Nombre AND 
		m.mat_descripcion = Material_Descripcion AND 
		m.mat_precio = Material_Precio
    WHERE Material_Tipo = 'Madera'
END
GO


CREATE PROCEDURE LOS_DBS.migrar_Tela
AS
BEGIN
	INSERT INTO LOS_DBS.Tela (mat_codigo,tel_color,tel_textura)
	SELECT DISTINCT
		m.mat_codigo,
		Maestra.Tela_Color,
		Maestra.Tela_Textura
	FROM Maestra 
	JOIN LOS_DBS.Material m ON 
		m.mat_nombre = Material_Nombre AND 
		m.mat_descripcion = Material_Descripcion AND 
		m.mat_precio = Material_Precio
    WHERE Material_Tipo = 'Tela'
END
GO


CREATE PROCEDURE LOS_DBS.migrar_Medida_Sillon
AS
BEGIN
	INSERT INTO LOS_DBS.Medida_Sillon (si_medida_alto, si_medida_ancho, si_medida_profundidad, si_medida_precio)
	SELECT DISTINCT 
		Sillon_Medida_Alto,
		Sillon_Medida_Ancho,
		Sillon_Medida_Profundidad,
		Sillon_Medida_Precio
	FROM Maestra
	WHERE Sillon_Medida_Alto IS NOT NULL AND 
		Sillon_Medida_Ancho IS NOT NULL AND 
		Sillon_Medida_Profundidad IS NOT NULL AND 
		Sillon_Medida_Precio IS NOT NULL
END
GO


CREATE PROCEDURE LOS_DBS.migrar_Modelo_Sillon
AS
BEGIN
	INSERT INTO LOS_DBS.Modelo_Sillon (si_modelo_codigo,si_modelo,si_modelo_descripcion,si_modelo_precio)
	SELECT DISTINCT 
		Sillon_Modelo_Codigo,
		Sillon_Modelo,
		Sillon_Modelo_Descripcion,
		Sillon_Modelo_Precio
	FROM Maestra 
	WHERE Sillon_Modelo_Codigo IS NOT NULL AND
		Sillon_Modelo IS NOT NULL AND
		Sillon_Modelo_Descripcion IS NOT NULL AND
		Sillon_Modelo_Precio IS NOT NULL
END
GO


CREATE PROCEDURE LOS_DBS.migrar_Sillon
AS
BEGIN 
	INSERT INTO LOS_DBS.Sillon (si_codigo,si_medida,si_modelo)
	SELECT DISTINCT
		Sillon_Codigo,
		med.si_medida,
		modsi.si_modelo_codigo
	FROM Maestra
	JOIN LOS_DBS.Medida_Sillon med ON 
		Sillon_Medida_Alto = med.si_medida_alto AND 
		Sillon_Medida_Ancho = med.si_medida_ancho AND 
		Sillon_Medida_Profundidad = med.si_medida_profundidad AND 
		Sillon_Medida_Precio = med.si_medida_precio
	JOIN LOS_DBS.Modelo_Sillon modsi ON 
		Sillon_Modelo_Codigo = modsi.si_modelo_codigo AND 
		Sillon_Modelo = modsi.si_modelo AND 
		Sillon_Modelo_Descripcion = modsi.si_modelo_descripcion AND 
		Sillon_Modelo_Precio = modsi.si_modelo_precio
	WHERE Sillon_Codigo IS NOT NULL
END
GO

CREATE PROCEDURE LOS_DBS.migrar_MaterialPorSillon
AS
BEGIN
    INSERT INTO LOS_DBS.MaterialPorSillon(mat_codigo, si_codigo)
    SELECT DISTINCT
        m.mat_codigo,
        s.si_codigo
    FROM Maestra
    JOIN LOS_DBS.Material m 
        ON m.mat_nombre = Material_Nombre 
        AND m.mat_descripcion = Material_Descripcion 
        AND m.mat_precio = Material_Precio
    JOIN LOS_DBS.Sillon s 
        ON s.si_codigo = Sillon_Codigo
	WHERE m.mat_codigo IS NOT NULL AND s.si_codigo IS NOT NULL
END
GO


CREATE PROCEDURE LOS_DBS.migrar_Compra
AS
BEGIN
    INSERT INTO LOS_DBS.Compra (comp_numero,comp_fecha,comp_total,comp_sucursal,comp_proveedor)
    SELECT DISTINCT
        m.Compra_Numero,
        m.Compra_Fecha,
        m.Compra_Total,
        s.sucu_nro,
        p.prov_cuit
    FROM Maestra m
	JOIN LOS_DBS.Sucursal s ON 
		m.Sucursal_NroSucursal = s.sucu_nro
	JOIN LOS_DBS.Proveedor p ON 
		m.Proveedor_Cuit = p.prov_cuit
    WHERE 
		m.Compra_Numero IS NOT NULL AND
		m.Compra_Fecha IS NOT NULL AND
        m.Compra_Total IS NOT NULL 
END
GO



CREATE PROCEDURE LOS_DBS.migrar_Detalle_Compra
AS
BEGIN
    INSERT INTO LOS_DBS.Detalle_Compra (det_comp_cantidad,det_comp_precio,det_comp_subtotal,comp_numero,mat_codigo)
    SELECT
        Detalle_Compra_Cantidad,
        Detalle_Compra_Precio,
        Detalle_Compra_SubTotal,
        Compra_Numero,
        mat.mat_codigo
    FROM Maestra
    JOIN LOS_DBS.Material mat ON 
        mat.mat_nombre = Material_Nombre AND
        mat.mat_descripcion = Material_Descripcion AND
        mat.mat_precio = Material_Precio
    WHERE Compra_Numero IS NOT NULL
END
GO

CREATE PROCEDURE LOS_DBS.migrar_DetallePedido
AS
BEGIN
	INSERT INTO LOS_DBS.DetallePedido (Detalle_Cantidad,Detalle_Precio,Detalle_Sillon,Detalle_SubTotal,Detalle_Pedido)
	SELECT DISTINCT
    ma.Detalle_Pedido_Cantidad,
    ma.Detalle_Pedido_Precio,
    si.si_codigo,
    ma.Detalle_Pedido_SubTotal,
    p.Pedido_Numero
	FROM Maestra ma
	JOIN LOS_DBS.Sillon si ON
		ma.Sillon_Codigo = si.si_codigo 
	JOIN LOS_DBS.Pedido p ON
	    p.Pedido_Numero = ma.Pedido_Numero
	WHERE 
	p.Pedido_Numero IS NOT NULL AND
	si.si_codigo IS NOT NULL AND
    ma.Detalle_Pedido_Precio IS NOT NULL AND
    ma.Detalle_Pedido_Cantidad IS NOT NULL AND
    ma.Detalle_Pedido_SubTotal IS NOT NULL
END
GO


CREATE PROCEDURE LOS_DBS.migrar_Factura
AS 
BEGIN
    INSERT INTO LOS_DBS.Factura(Factura_Numero,Factura_Fecha,Factura_Cliente,Factura_Sucursal)
    SELECT DISTINCT
        ma.Factura_Numero,
        ma.Factura_Fecha,
        cli.Cliente_Codigo,
        s.sucu_nro
    FROM gd_esquema.Maestra ma
    JOIN LOS_DBS.Cliente cli ON cli.Cliente_Dni = ma.Cliente_Dni
	JOIN LOS_DBS.Sucursal s ON s.sucu_nro = ma.Sucursal_NroSucursal
    WHERE 
		ma.Factura_Numero IS NOT NULL AND
		cli.Cliente_Codigo IS NOT NULL AND
		s.sucu_nro IS NOT NULL AND
        ma.Factura_Fecha IS NOT NULL
END
GO

CREATE PROCEDURE LOS_DBS.migrar_DetalleFactura
AS 
BEGIN
    INSERT INTO LOS_DBS.DetalleFactura (Detalle_Cantidad,Detalle_Precio,Detalle_SubTotal,Detalle_Factura,Detalle_Pedido)
    SELECT DISTINCT
        ma.Detalle_Factura_Cantidad,
        ma.Detalle_Factura_Precio,
        ma.Detalle_Factura_SubTotal,
        f.Factura_Numero,
        dp.Detalle_Codigo
    FROM gd_esquema.Maestra ma
    INNER JOIN LOS_DBS.DetallePedido dp ON 
        dp.Detalle_Pedido = ma.Pedido_Numero AND
        dp.Detalle_Cantidad = ma.Detalle_Factura_Cantidad AND
        dp.Detalle_Precio = ma.Detalle_Factura_Precio
    INNER JOIN LOS_DBS.Factura f ON 
        f.Factura_Numero = ma.Factura_Numero
    WHERE
        ma.Detalle_Factura_Cantidad IS NOT NULL AND 
        ma.Detalle_Factura_Precio IS NOT NULL AND 
        ma.Detalle_Factura_SubTotal IS NOT NULL;
END
GO

create procedure LOS_DBS.migrar_Envio
as 
begin
    INSERT INTO LOS_DBS.Envio(Envio_Numero,Envio_Fecha_Programada,Envio_Fecha,Envio_Importe_Traslado,Envio_Importe_Subida,Envio_Total,Envio_Factura)
	SELECT DISTINCT
		ma.Envio_Numero,
		ma.Envio_Fecha_Programada,
		ma.Envio_Fecha,
		ma.Envio_ImporteTraslado as Envio_Importe_Traslado,
		ma.Envio_importeSubida as Envio_Importe_Subida,
		ma.Envio_Total,
		f.Factura_Numero
	from gd_esquema.Maestra ma
	JOIN Factura f ON f.Factura_Numero = ma.Factura_Numero
	WHERE 
		ma.Envio_Numero IS NOT NULL AND
		ma.Envio_Fecha_Programada IS NOT NULL
END
GO


--Indices

CREATE NONCLUSTERED INDEX IX_Material_Nombre_Descripcion_Precio ON 
	LOS_DBS.Material (mat_nombre, mat_descripcion, mat_precio)

CREATE NONCLUSTERED INDEX IX_DetallePedido_Pedido_Cantidad_Precio ON 
	LOS_DBS.DetallePedido (Detalle_Pedido, Detalle_Cantidad, Detalle_Precio)

CREATE NONCLUSTERED INDEX IX_Factura_Numero ON 
	LOS_DBS.Factura (Factura_Numero)

CREATE NONCLUSTERED INDEX IX_Cliente_Dni ON 
	LOS_DBS.Cliente (Cliente_Dni)

--Ejecuciones

EXEC LOS_DBS.migrar_Provincia
EXEC LOS_DBS.migrar_Localidad
EXEC LOS_DBS.migrar_Direccion
EXEC LOS_DBS.migrar_EstadoPedido
EXEC LOS_DBS.migrar_Sucursal
EXEC LOS_DBS.migrar_Cliente
EXEC LOS_DBS.migrar_Pedido
EXEC LOS_DBS.migrar_Proveedor
EXEC LOS_DBS.migrar_PedidoCancelacion
EXEC LOS_DBS.migrar_Material
EXEC LOS_DBS.migrar_Relleno
EXEC LOS_DBS.migrar_Maderas
EXEC LOS_DBS.migrar_Tela
EXEC LOS_DBS.migrar_Medida_Sillon
EXEC LOS_DBS.migrar_Modelo_Sillon
EXEC LOS_DBS.migrar_Sillon
EXEC LOS_DBS.migrar_MaterialPorSillon
EXEC LOS_DBS.migrar_Compra
EXEC LOS_DBS.migrar_Detalle_Compra
EXEC LOS_DBS.migrar_DetallePedido
EXEC LOS_DBS.migrar_Factura
EXEC LOS_DBS.migrar_DetalleFactura
EXEC LOS_DBS.migrar_Envio

