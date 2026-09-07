vim.api.nvim_create_autocmd("LspAttach", {
    group = vim.api.nvim_create_augroup("proj.lspIgnoreFiles", {}),
    callback = function(e)
        vim.lsp.get_client_by_id(e.data.client_id):stop()
    end,
    pattern = vim.fn.getcwd().."/config/*"
})
