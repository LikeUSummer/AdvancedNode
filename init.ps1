if (-not (Test-Path node)) {
    Expand-Archive -Path node.zip -DestinationPath node
}
