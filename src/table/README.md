This table browser is adapted from ParquetViewer commit
`f4d38247ee72948d46c0fb11f7825b7225b54fdc`.

The original file tabs, explorer, paging, search, quick filters, and visual
filter builder are embedded in ParquetPlot. `TableBrowser` replaces the original
standalone main window; `FileTab::showFromRow` connects plot X positions to
source rows. The query engine remains the DuckDB CLI.

Copyright (C) 2026 Furkan Selim Cetin. Original GPL-3.0-or-later notices are
retained in each source file.
