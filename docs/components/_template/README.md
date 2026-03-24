# Component Docs Template

Use this folder as a starting point when adding a new component's documentation.

## Recommended Structure

Create `docs/components/<component_name>/` with:

- `index.md` (component overview)
- `getting-started.md`
- `config.md`
- `sensors.md` (or equivalent entities/features page)
- `compatibility.md`
- `troubleshooting.md`
- `architecture.md`
- `roadmap.md`
- `release-notes.md`

## Minimal Starter Content

### `index.md`

- What the component does
- Main features
- Quick links to all component pages

### `getting-started.md`

- Prerequisites
- `external_components` installation snippet
- Basic YAML example

### `config.md`

- Full YAML example
- Option reference table
- Actions/services (if any)

## MkDocs Navigation Snippet

Add this block under `nav > Components` in `mkdocs.yaml` (replace names/paths):

```yaml
- <Component Display Name>:
    - Overview: components/<component_name>/index.md
    - Getting Started: components/<component_name>/getting-started.md
    - Configuration: components/<component_name>/config.md
    - Sensors: components/<component_name>/sensors.md
    - Compatibility: components/<component_name>/compatibility.md
    - Troubleshooting: components/<component_name>/troubleshooting.md
    - Architecture: components/<component_name>/architecture.md
    - Roadmap: components/<component_name>/roadmap.md
    - Release Notes: components/<component_name>/release-notes.md
```

## Optional Doxygen API Docs (Per Component)

If the component has C/C++ sources, generate API docs into the component docs folder:

- Doxygen output path: `docs/components/<component_name>/api`
- MkDocs nav entry:

```yaml
- Architecture:
    - Overview: components/<component_name>/architecture.md
    - C++ Reference: components/<component_name>/api/index.html
```

This keeps API docs scoped per component instead of mixing all components together.

## Maintenance Checklist

- Keep links relative within each component folder.
- Update `README.md` with a short component entry when adding a new component.
- Add/update component section in `docs/index.md`.
- Add component navigation entries to `mkdocs.yaml`.
